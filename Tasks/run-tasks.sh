echo "📦 Starting the database container..."
docker compose up -d db

echo "⏳ Waiting for the database to be ready..."
until docker exec $(docker-compose ps -q db) pg_isready > /dev/null 2>&1; do
  sleep 1
done

echo "✅ Database is ready."

echo "🚀 Starting the Tasks app..."
docker compose run --rm --entrypoint ./build/Tasks app

read -p "🛑 Do you want to stop the database container now? [y/N]: " answer
if [[ "$answer" =~ ^[Yy]$ ]]; then
  echo "🧹 Stopping and cleaning up database container..."
  docker compose down
else
  echo "ℹ Database is still running. You can stop it manually with: docker compose down"
fi