FROM debian:bookworm-slim

WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends g++ nginx \
    && rm -f /etc/nginx/sites-enabled/default \
    && rm -rf /var/lib/apt/lists/*

COPY main.cpp .
COPY templates ./templates
COPY nginx.conf /etc/nginx/conf.d/default.conf

RUN g++ -std=c++17 -O2 -o app main.cpp

EXPOSE 80

CMD ["sh", "-c", "./app & nginx -g 'daemon off;'"]
