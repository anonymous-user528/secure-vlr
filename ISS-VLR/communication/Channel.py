import socket
import pickle
from numpy.lib.function_base import select
import os

class Channel:
    def __init__(self, host, port):
        self.host = host
        self.port = port
    
    def create_socket(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, True)
        s.bind((self.host, self.port))
        return s

    def listen(self, max_client_num):
        if self.s is None:
            raise ValueError("socket doesn't be created!")
        self.s.listen(max_client_num)
    
    def connect(self, host, port):
        self.s.connect((host, port))

    def accept(self):
        return self.s.accept()

    def send(self, conn, msg):
        self.check_status(conn)
        byteStream = pickle.dumps(msg)
        length = len(byteStream)
        conn.sendall(bytes(f"{length:<16}", 'utf-8'))
        conn.sendall(byteStream)

    def recv(self, conn, bufsize=1024):
        self.check_status(conn)
        msg = conn.recv(bufsize)
        length = int(msg[:16])
        full_msg = b'' + msg[16:]
        now_size = len(full_msg)
        while now_size < length:
            more_msg = conn.recv(length - now_size)
            full_msg += more_msg
            now_size += len(more_msg)
        
        return pickle.loads(full_msg)

    def close(self):
        self.s.close()

    def check_status(self, conn):
        conn_status = conn.getsockopt(socket.SOL_SOCKET, socket.SO_ERROR)
        assert conn_status == 0


class ActiveChannel(Channel):
    def __init__(self, host, port, conn_num=1):
        super().__init__(host, port)
        self.s = self.create_socket()
        self.conns = []
        max_conn_num = conn_num + 1
        self.listen(max_conn_num)

        for _ in range(conn_num):
            client_socket, _ = self.accept()
            self.conns.append(client_socket)
        
    def send_to_passive(self, data, index=-1):
        if index == -1:
            conns = self.conns
        elif index < len(self.conns) and index >= 0:
            conns = [self.conns[index]]
        else:
            raise ValueError('the index is out of range, there are {} conns, while the index given is {}'.format(len(self.conns), index))
        
        for conn in conns:
            self.send(conn, data)
    
    def recv_from_passive(self, index=-1):
        data = None

        if index == -1:
            data = []
            for conn in self.conns:
                data.append(self.recv(conn))
        elif index < len(self.conns) and index >= 0:
            data = self.recv(self.conns[index])
        else:
            raise ValueError('the index is out of range, there are {} conns, while the index given is {}'.format(len(self.conns), index))

        if len(data) == 1:
            return data[0]
        else:
            return data

class PassiveChannel(Channel):
    def __init__(self, host, port, ac_host, ac_port):
        super().__init__(host, port)
        self.s = self.create_socket()
        self.connect(ac_host, ac_port)
    
    def send_to_active(self, msg):
        self.send(self.s, msg)
    
    def recv_from_active(self):
        msg = self.recv(self.s)
        return msg