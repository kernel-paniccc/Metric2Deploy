FROM debian:bookworm-slim

WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends g++ nginx libcpp-httplib-dev \
    && rm -f /etc/nginx/sites-enabled/default \
    && rm -rf /var/lib/apt/lists/*

COPY main.cpp .
COPY templates ./templates
COPY nginx.conf /etc/nginx/conf.d/default.conf

RUN useradd -m -u 1001 appuser

RUN g++ -std=c++17 -O2 -o app main.cpp -lcpp-httplib

RUN chown -R appuser:appuser /app

USER appuser

EXPOSE 80

CMD ["sh", "-c", "./app & nginx -g 'daemon off;'"]
