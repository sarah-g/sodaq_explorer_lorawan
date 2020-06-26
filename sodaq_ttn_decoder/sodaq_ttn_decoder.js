/*
  Simple decoder for TTN to decode 4 byte payload containing 2 values from Sodaq Explorer boards
  
  Author: Sarah Gallacher
*/

function DecodePayload(data){
    var obj = new Object();
    
    //data structure is 4 bytes
    //bytes 1 & 2 = reading 1
    //bytes 3 & 4 = reading 2

    //decode reading 1
    //shift first byte 8 bits to the left and then add second byte to get final number
    obj.reading1 = (data[0] << 8) + data[1];
    
    //decode reading 2
    //shift first byte 8 bits to the left and then add second byte to get final number
    obj.reading2 = (data[2] << 8) + data[3];
    
    return obj;
}

function Decoder(bytes, port) {
  return DecodePayload(bytes);
}