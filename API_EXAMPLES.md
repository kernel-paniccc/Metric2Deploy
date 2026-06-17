# API Examples

Examples of how to use the Metric2Deploy API endpoints.

## Health Check

Check if the service is running:

```bash
curl http://localhost/health
```

**Response:**
```json
{"status":"ok"}
```

---

## Get Time

Get current UTC time:

```bash
curl http://localhost/time
```

**Response:**
```json
{
  "iso8601": "2024-06-18T12:00:00Z",
  "unix": 1718712000
}
```

---

## Random Number

Generate a random number within a range:

```bash
curl "http://localhost/random?min=1&max=100"
```

**Response:**
```json
{"value":42}
```

**With custom range:**
```bash
curl "http://localhost/random?min=10&max=20"
```

---

## Echo

Send a message and get it back:

```bash
curl "http://localhost/echo?message=hello"
```

**Response:**
```json
{"echo":"hello"}
```

---

## System Info

Get information about the server environment:

```bash
curl http://localhost/info
```

**Response:**
```json
{
  "hostname": "app-pod-123",
  "cwd": "/app",
  "env": {
    "PATH": "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin",
    "HOME": "/root"
  }
}
```

---

## Main Page

Open the main HTML page in your browser:

```
http://localhost/
```

Or via curl:

```bash
curl http://localhost/
```

---

## Error Examples

### Wrong HTTP method (POST instead of GET):

```bash
curl -X POST http://localhost/health
```

**Response:**
```json
{"error":"Method Not Allowed"}
```
Status: `405 Method Not Allowed`

---

### Invalid endpoint:

```bash
curl http://localhost/invalid
```

**Response:**
```json
{"error":"Not Found"}
```
Status: `404 Not Found`

---

## Testing with Docker

If the application is running in Docker:

```bash
# Build the image
docker build -t metric2deploy ./src

# Run the container
docker run -d -p 80:80 --name metric2deploy metric2deploy

# Test endpoints
curl http://localhost/health
curl http://localhost/time

# Stop the container
docker stop metric2deploy
docker rm metric2deploy
```

---

## Testing in k3s Cluster

After deploying to k3s:

```bash
# Get the service IP
kubectl get services

# Test via NodePort or Ingress
curl http://<cluster-ip>/health
curl http://<cluster-ip>/time
```

---

**All endpoints support only GET method.**
