FROM python:3.13-slim

ENV PYTHONDONTWRITEBYTECODE=1 PYTHONUNBUFFERED=1
WORKDIR /app
COPY requirements-remote.txt ./
RUN pip install --no-cache-dir -r requirements-remote.txt && useradd --create-home --uid 10001 smartagri
COPY smartagri ./smartagri
COPY wsgi.py ./
RUN chown -R smartagri:smartagri /app
USER smartagri
EXPOSE 8000
CMD ["gunicorn", "--workers", "2", "--bind", "0.0.0.0:8000", "wsgi:app"]
