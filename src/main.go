package main

import (
	"bytes"
	"context"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"runtime"
	"strconv"
	"strings"
	"syscall"
	"time"

	"github.com/gin-gonic/gin"
)

type pageData struct {
	Title       string
	Host        string
	CPU         int
	Memory      string
	Uptime      string
	StartedAt   string
	Environment string
	Version     string
}

func main() {
	gin.SetMode(gin.ReleaseMode)

	router := gin.New()
	router.SetTrustedProxies(nil)
	router.Use(gin.Recovery())
	router.Use(requestLogger())
	router.Use(securityHeaders())

	router.LoadHTMLGlob("templates/*.html")

	router.GET("/", func(c *gin.Context) {
		data := pageData{
			Title:       "Metric2Deploy",
			Host:        hostname(),
			CPU:         runtime.NumCPU(),
			Memory:      memoryUsedMB(),
			Uptime:      uptimeMinutes(),
			StartedAt:   time.Now().UTC().Format(time.RFC3339),
			Environment: getenvDefault("APP_ENV", "production"),
			Version:     getenvDefault("APP_VERSION", "dev"),
		}

		c.HTML(http.StatusOK, "index.html", data)
	})

	router.GET("/health", func(c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{"status": "ok"})
	})

	router.GET("/api/runtime", func(c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{
			"service":     "metric2deploy",
			"hostname":    hostname(),
			"cpu_cores":   runtime.NumCPU(),
			"memory_bytes": memoryUsedBytes(),
			"uptime_sec":  uptimeSeconds(),
			"go_version":  runtime.Version(),
			"environment": getenvDefault("APP_ENV", "production"),
			"version":     getenvDefault("APP_VERSION", "dev"),
		})
	})

	router.GET("/metrics", func(c *gin.Context) {
		var out bytes.Buffer
		out.WriteString("# HELP metric2deploy_uptime_seconds Uptime in seconds\n")
		out.WriteString("# TYPE metric2deploy_uptime_seconds gauge\n")
		out.WriteString(fmt.Sprintf("metric2deploy_uptime_seconds %.2f\n", uptimeSeconds()))
		out.WriteString("# HELP metric2deploy_memory_used_bytes Memory used by the Go process\n")
		out.WriteString("# TYPE metric2deploy_memory_used_bytes gauge\n")
		out.WriteString(fmt.Sprintf("metric2deploy_memory_used_bytes %d\n", memoryUsedBytes()))
		out.WriteString("# HELP metric2deploy_cpu_cores CPU cores visible to the container\n")
		out.WriteString("# TYPE metric2deploy_cpu_cores gauge\n")
		out.WriteString(fmt.Sprintf("metric2deploy_cpu_cores %d\n", runtime.NumCPU()))

		c.Data(http.StatusOK, "text/plain; version=0.0.4; charset=utf-8", out.Bytes())
	})

	server := &http.Server{
		Addr:              "127.0.0.1:8000",
		Handler:           router,
		ReadHeaderTimeout: 5 * time.Second,
		ReadTimeout:       10 * time.Second,
		WriteTimeout:      15 * time.Second,
		IdleTimeout:       60 * time.Second,
	}

	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	go func() {
		log.Printf("listening on %s", server.Addr)
		if err := server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			log.Fatalf("server failed: %v", err)
		}
	}()

	<-ctx.Done()
	shutdownCtx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	if err := server.Shutdown(shutdownCtx); err != nil {
		log.Printf("graceful shutdown failed: %v", err)
		return
	}

	log.Print("server stopped cleanly")
}

func requestLogger() gin.HandlerFunc {
	return func(c *gin.Context) {
		started := time.Now()
		c.Next()
		log.Printf("%s %s -> %d (%s)", c.Request.Method, c.Request.URL.Path, c.Writer.Status(), time.Since(started).Round(time.Millisecond))
	}
}

func securityHeaders() gin.HandlerFunc {
	return func(c *gin.Context) {
		c.Header("X-Content-Type-Options", "nosniff")
		c.Header("X-Frame-Options", "DENY")
		c.Header("Referrer-Policy", "no-referrer")
		c.Header("Content-Security-Policy", "default-src 'self'; script-src 'self' https://cdn.tailwindcss.com; style-src 'self' 'unsafe-inline' https://cdn.tailwindcss.com; img-src 'self' data:; font-src 'self' https:; connect-src 'self'; base-uri 'self'; form-action 'self'; frame-ancestors 'none'")
		c.Next()
	}
}

func hostname() string {
	host, err := os.Hostname()
	if err != nil || host == "" {
		return "local"
	}

	return host
}

func getenvDefault(key, fallback string) string {
	value := strings.TrimSpace(os.Getenv(key))
	if value == "" {
		return fallback
	}

	return value
}

func uptimeSeconds() float64 {
	data, err := os.ReadFile("/proc/uptime")
	if err != nil {
		return 0
	}

	fields := strings.Fields(string(data))
	if len(fields) == 0 {
		return 0
	}

	seconds, err := strconv.ParseFloat(fields[0], 64)
	if err != nil {
		return 0
	}

	return seconds
}

func uptimeMinutes() string {
	return fmt.Sprintf("%.0f min", uptimeSeconds()/60)
}

func memoryUsedBytes() uint64 {
	var stats runtime.MemStats
	runtime.ReadMemStats(&stats)
	return stats.Alloc
}

func memoryUsedMB() string {
	return fmt.Sprintf("%.1f MB", float64(memoryUsedBytes())/1024/1024)
}
