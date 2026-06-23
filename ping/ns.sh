#!/bin/bash

NS1=ns1
NS2=ns2
VETH1=veth1
VETH2=veth2
IP1=10.0.0.1
IP2=10.0.0.2

setup() {
    ip netns add $NS1                                          # create network namespace ns1
    ip netns add $NS2                                          # create network namespace ns2
    ip link add $VETH1 type veth peer name $VETH2              # create veth pair: veth1 <-> veth2
    ip link set $VETH1 netns $NS1                              # move veth1 into ns1
    ip link set $VETH2 netns $NS2                              # move veth2 into ns2
    ip netns exec $NS1 ip addr add $IP1/24 dev $VETH1          # assign 10.0.0.1/24 to veth1 inside ns1
    ip netns exec $NS2 ip addr add $IP2/24 dev $VETH2          # assign 10.0.0.2/24 to veth2 inside ns2
    ip netns exec $NS1 ip link set $VETH1 up                   # bring veth1 up inside ns1
    ip netns exec $NS2 ip link set $VETH2 up                   # bring veth2 up inside ns2
    ip netns exec $NS1 ip link set lo up                       # bring loopback up inside ns1
    ip netns exec $NS2 ip link set lo up                       # bring loopback up inside ns2
}

clean() {
    ip netns del $NS1 2>/dev/null || true
    ip netns del $NS2 2>/dev/null || true
}

ping_test() {
    ip netns exec $NS1 ./bin/ping $IP2
}

pong_server() {
    ip netns exec $NS2 ./bin/pong
}

case "$1" in
    setup) setup ;;
    clean) clean ;;
    ping)  ping_test ;;
    pong)  pong_server ;;
    *)
        echo "Usage: $0 {setup|clean|ping|pong}"
        exit 1
        ;;
esac
