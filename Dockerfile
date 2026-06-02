FROM gcc:13

RUN apt-get update && apt-get install -y cmake && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -S . -B build && cmake --build build

ENTRYPOINT ["./build/Compiler"]
