FROM golang:1.23-bookworm AS builder

WORKDIR /src

COPY go.mod go.sum ./
RUN go mod download

COPY main.go ./
COPY templates ./templates

RUN CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build -ldflags='-s -w' -o /out/app ./main.go

FROM debian:bookworm-slim

WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends ca-certificates nginx libcap2-bin \
    && rm -f /etc/nginx/sites-enabled/default \
    && sed -i '/^user /d' /etc/nginx/nginx.conf \
    && sed -i '/^pid /d' /etc/nginx/nginx.conf \
    && rm -rf /var/lib/apt/lists/* \
    && useradd -m -u 1001 appuser

COPY --from=builder /out/app /app/app
COPY templates ./templates
COPY nginx.conf /etc/nginx/conf.d/default.conf

RUN setcap 'cap_net_bind_service=+ep' /usr/sbin/nginx \
    && chown -R appuser:appuser /app \
    && chown -R appuser:appuser /etc/nginx /var/log/nginx /var/lib/nginx

USER 1001

ENV APP_ENV=production

EXPOSE 80

CMD ["sh", "-c", "/app/app & nginx -g 'pid /tmp/nginx.pid; daemon off;'"]
