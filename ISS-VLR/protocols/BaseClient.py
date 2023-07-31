from communication.Channel import Channel, ActiveChannel, PassiveChannel
from logger.Logger import Logger
import abc
import numpy as np
from utils import load_data_from_csv
class BaseClient(metaclass=abc.ABCMeta):
    def __init__(self, config):
        self.host = config['host']
        self.port = config['port']
        self.max_client_num = config['max_communicate_num']
        self.random_seed = config['random_seed']
        self.epochs = config['epochs']
        self.learning_rate = config['learning_rate']
        #self.regularization = config['regularization']
        self.batch_size = config['batch_size']
        self.train_data_filename = config['train_data_filename']
        self.test_data_filename = config['test_data_filename']
        self.val_data_filename = config['val_data_filename']
        self.max_threads = config['max_threads']
        self.verbose = config['verbose']
        self.log_path = config['logging_path']
        self.logger = Logger(self.log_path, self.verbose)
        self.ch = Channel(self.host, self.port)

class BaseActiveClient(BaseClient):
    def __init__(self, config):
        super().__init__(config)
        self.train_label_filename = config['train_label_filename']
        self.test_label_filename = config['test_label_filename']
        self.val_label_filename = config['val_label_filename']
        self.ch = ActiveChannel(self.host, self.port, self.max_client_num)

    @abc.abstractclassmethod
    def fit(self, X, Y):
        pass

    @abc.abstractclassmethod
    def predict(self, X):
        pass

    def generate_batch_idx(self, X):
        X_index = np.array(range(X.shape[0]))
        np.random.shuffle(X_index)
        X_index = X_index.tolist()

        batch_idx = np.array_split(X_index, len(X_index) // self.batch_size)
        return batch_idx

    def load_data(self):
        train_data = None
        val_data = None
        train_label = None
        val_label = None
        
        if self.train_data_filename is not None:
            train_data = load_data_from_csv(self.train_data_filename)
            self.logger.saveMsg("Load training data.")
        else:
            raise ValueError("train_data_filename cannot be empty!")
        if self.val_data_filename is not None:
            val_data = load_data_from_csv(self.val_data_filename)
        if self.train_label_filename is not None:
            train_label = load_data_from_csv(self.train_label_filename)
        else:
            raise ValueError("train_label_filename cannot be empty!")
        if self.val_label_filename is not None:
            val_label = load_data_from_csv(self.val_label_filename)

        return np.array(train_data), np.array(train_label), np.array(val_data), np.array(val_label)

    def sigmoid(self, x):
        x *= -1
        y = np.exp(x)
        y += 1
        y = np.reciprocal(y)
        return y

class BasePassiveClient(BaseClient):
    def __init__(self, config):
        super().__init__(config)
        self.ac_host = config['ac_host']
        self.ac_port = config['ac_port']
        self.ch = PassiveChannel(self.host, self.port, self.ac_host, self.ac_port)
    
    @abc.abstractclassmethod
    def fit(self, X):
        pass

    @abc.abstractclassmethod
    def predict(self, X):
        pass

    def load_data(self):
        train_data = None
        val_data = None
        if self.train_data_filename is not None:
            train_data = load_data_from_csv(self.train_data_filename)
            self.logger.saveMsg("Load training data.")
        else:
            raise ValueError("train_data_filename cannot be empty!")
        
        if self.val_data_filename is not None:
            val_data = load_data_from_csv(self.val_data_filename)

        return np.array(train_data), np.array(val_data)