# Frontend

The main page is a Jinja2-like template with variables filled by C++ backend.

## Template Variables
- `{{title}}` — application title
- `{{host}}` — server hostname
- `{{cpu}}` — CPU cores count
- `{{memory}}` — used memory
- `{{uptime}}` — service uptime

## Design
- Tailwind CSS via CDN
- Dark theme with gradients
- Responsive layout
- Health check badge
