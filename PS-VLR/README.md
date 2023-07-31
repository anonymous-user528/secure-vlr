# MS-MLR

## dependencies
`boost 1.80.0`
`gmp-6.1.2`
`ntl-10.5.0`  
`openfhe`  

## build
`mkdir build`  
`cd build`  
`cmake ..`  
`make`  

## run
`./build/test --client-id 0 --client-num 4 --network-file network_4.txt --data-file data/chess/client_0.txt` 

`./build/test --client-id 1 --client-num 4 --network-file network_4.txt --data-file data/chess/client_1.txt` 

`./build/test --client-id 2 --client-num 4 --network-file network_4.txt --data-file data/chess/client_2.txt`

`./build/test --client-id 3 --client-num 4 --network-file network_4.txt --data-file data/chess/client_3.txt`
