#!/bin/bash

# =============================================
# Backup script for users.db
# Зберігає копію бази даних з міткою дати
# Запускати: ./backup.sh  або через cron
# =============================================

DB_PATH="./auth_service/db/users.db"
BACKUP_DIR="./backups"
DATE=$(date +"%Y-%m-%d_%H-%M")
BACKUP_FILE="$BACKUP_DIR/users_$DATE.db"
KEEP_DAYS=7  # Зберігати копії за останні 7 днів

# Перевірка чи існує папка backups
mkdir -p "$BACKUP_DIR"

# Перевірка чи існує файл БД
if [ ! -f "$DB_PATH" ]; then
    echo "[ERROR] Файл бази даних не знайдено: $DB_PATH"
    exit 1
fi

# Копіювання файлу
cp "$DB_PATH" "$BACKUP_FILE"

if [ $? -eq 0 ]; then
    SIZE=$(du -h "$BACKUP_FILE" | cut -f1)
    echo "[OK] Бекап створено: $BACKUP_FILE ($SIZE)"
else
    echo "[ERROR] Не вдалося створити бекап!"
    exit 1
fi

# Видалення старих бекапів (старше KEEP_DAYS днів)
DELETED=$(find "$BACKUP_DIR" -name "users_*.db" -mtime +$KEEP_DAYS -print -delete | wc -l | tr -d ' ')
if [ "$DELETED" -gt 0 ]; then
    echo "[CLEAN] Видалено старих бекапів: $DELETED"
fi

echo "[INFO] Всього бекапів збережено: $(ls $BACKUP_DIR/users_*.db 2>/dev/null | wc -l | tr -d ' ')"
