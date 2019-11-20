FROM alpine AS builder
RUN apk update
run apk add g++
run apk add make
run apk add boost
run apk add boost-dev
COPY . /SplunkEventGenerator
WORKDIR /SplunkEventGenerator/Release
RUN make

FROM alpine
# RUN apk update 
COPY --from=builder /SplunkEventGenerator/Release/SplunkEventGenerator /bin
COPY --from=builder /usr/lib/libstdc++.so.6 /usr/lib
COPY --from=builder /usr/lib/libgcc_s.so.1 /usr/lib
WORKDIR bin
CMD ["./SplunkEventGenerator"]
