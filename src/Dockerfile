FROM debian:bookworm-slim

WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends g++ nginx libcpp-httplib-dev libcap2-bin \
    && rm -f /etc/nginx/sites-enabled/default \
    && sed -i '/^pid /d' /etc/nginx/nginx.conf \
    && rm -rf /var/lib/apt/lists/*

COPY main.cpp system_utils.h system_utils.cpp .
COPY templates ./templates
COPY nginx.conf /etc/nginx/conf.d/default.conf

RUN useradd -m -u 1001 appuser

RUN g++ -std=c++17 -O2 -o app main.cpp system_utils.cpp -lcpp-httplib && \
    setcap 'cap_net_bind_service=+ep' /usr/sbin/nginx

RUN chown -R appuser:appuser /app && \
    chown -R appuser:appuser /etc/nginx /var/log/nginx /var/lib/nginx

USER 1001

EXPOSE 80

CMD ["sh", "-c", "./app & nginx -g 'pid /tmp/nginx.pid; daemon off;'"]
