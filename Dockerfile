FROM gcc:latest

WORKDIR /app

COPY . .

RUN gcc -c db/sqlite3.c -o db/sqlite3.o
RUN g++ -std=c++17 main.cpp db/sqlite3.o -o app -lpthread -ldl

CMD ["./app"]
