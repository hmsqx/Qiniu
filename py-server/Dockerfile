# Python app Dockerfile
FROM python:3.11-slim

ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    PIP_NO_CACHE_DIR=1

# Set working directory to /app
WORKDIR /app

# System deps for opencv and PIL
RUN apt-get update \
     && apt-get install -y --no-install-recommends -o Acquire::Retries=3 --fix-missing \
         build-essential libgl1 libglib2.0-0 \
     && rm -rf /var/lib/apt/lists/*

# Copy requirements first for better caching
COPY requirements.txt ./requirements.txt
RUN pip install --no-cache-dir -r requirements.txt

# Copy source
COPY . .


COPY docker-entrypoint.sh /usr/local/bin/entry.sh
RUN chmod +x /usr/local/bin/entry.sh

EXPOSE 8090

CMD ["/usr/local/bin/entry.sh"]
