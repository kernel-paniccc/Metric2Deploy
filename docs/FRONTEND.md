# Frontend Documentation

## Overview
Главная страница приложения Metric2Deploy — это современный HTML-шаблон с Tailwind CSS, который отображает информацию о сервисе и доступных endpoint'ах.

## Template Variables
C++ backend подставляет следующие переменные в шаблон:
- `{{title}}` — заголовок приложения
- `{{host}}` — hostname сервера
- `{{cpu}}` — количество CPU ядер
- `{{memory}}` — используемая память
- `{{uptime}}` — время работы сервиса

## Features
- **Dark theme UI** — современный тёмный интерфейс с градиентами
- **Health check badge** — индикатор доступности сервиса
- **System metrics** — отображение метрик системы в реальном времени
- **Responsive design** — адаптивная вёрстка для мобильных и десктопов
- **Tailwind CSS** — стилизация через CDN (без сборки)

## Files
- `src/templates/index.html` — основной шаблон страницы

## Design Decisions
- Использован Tailwind CSS via CDN для быстрого прототипирования без build step
- Тёмная тема для developer-friendly интерфейса
- Градиенты и анимации для современного вида
- Карточки с метриками для наглядности
