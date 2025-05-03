#!/bin/sh
set -e
host="$1"
shift
cmd="$@"

until pg_isready -h "$host" -p 5432; do
  sleep 1
done

exec $cmd
