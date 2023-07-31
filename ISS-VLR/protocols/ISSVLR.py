from .BaseClient import BaseActiveClient, BasePassiveClient
import numpy as np
from time import time
from tqdm import tqdm
from joblib import Parallel, delayed
from sklearn.metrics import accuracy_score

class ISSVLRActiveClient(BaseActiveClient):
    def __init__(self, config):
        super().__init__(config)
        self.logger.saveMsg("Active party start.")
        self.logger.saveMsg("Create Channel for Communication.")

    def fit(self, X=None, Y=None):
        if X is None:
            X, Y, val_X, val_Y = self.load_data()
        
        assert X is not None
        assert Y is not None

        n = X.shape[0]
        m_b = X.shape[1]

        self.logger.saveMsg("Start training.")
        start_time = time()

        theta_b = np.random.normal(0, 1, m_b)
        if self.max_client_num == 1:
            phi_list = [1]
        else:
            phi_list = []
            for i in range(self.max_client_num):
                phi_list.append([1])

        for e in range(self.epochs):
            self.logger.saveMsg("Epoch: {}.".format(e))
            batch_idx = self.generate_batch_idx(X)
            self.ch.send_to_passive(batch_idx)
            self.logger.saveMsg("Batch dividing is over.")
            for i in tqdm(range(len(batch_idx))):
                s1 = time()
                idx_list = batch_idx[i]
                batch_X = X[idx_list].astype(float)
                batch_Y = Y[idx_list].astype(float)

                z_b = np.dot(batch_X, theta_b)
                z_a = self.ch.recv_from_passive()
                if len(z_a) > 1:
                    z = z_b
                    for idx, _z in enumerate(z_a):
                        _z /= phi_list[idx][e*len(batch_idx)+i]
                        z += _z
                else:
                    z_a /= phi_list[e*len(batch_idx)+i]
                    z = z_a + z_b
                y_pred = self.sigmoid(z)
                delta =  y_pred - np.squeeze(batch_Y)
                sigma_i = np.random.normal(0, 1)
                tor_i = np.random.normal(0, 1)

                masked_delta = sigma_i * delta + tor_i
                self.ch.send_to_passive(masked_delta)

                grad_b = np.dot(batch_X.T, delta)
                theta_b = theta_b - self.learning_rate * grad_b 
                
                if self.max_client_num == 1:
                    masked_grad_S_a = self.ch.recv_from_passive()
                    masked_S_a = self.ch.recv_from_passive()
                    masked_grad = (masked_grad_S_a - tor_i*masked_S_a) / sigma_i
                    mu_i = np.random.normal(0, 1, masked_grad.shape[0])
                    masked_grad = self.learning_rate * phi_list[e*len(batch_idx)+i] * masked_grad + mu_i
                    self.ch.send_to_passive(masked_grad)

                    masked_theta_a = self.ch.recv_from_passive()
                    masked_theta_a = (masked_theta_a + mu_i) / phi_list[e*len(batch_idx)+i]
                    phi_i = np.random.normal(0, 1)
                    while phi_i == 0:
                        phi_i = np.random.normal(0, 1)
                    masked_theta_a *= phi_i
                    phi_list.append(phi_i)
                    self.ch.send_to_passive(masked_theta_a)
                else:
                    def _update_grad(_phi, index):
                        masked_grad_S_a, masked_S_a = self.ch.recv_from_passive(index=index)
                        masked_grad = (masked_grad_S_a - tor_i*masked_S_a) / sigma_i
                        mu_i = np.random.normal(0, 1, masked_grad.shape[0])
                        masked_grad = self.learning_rate * _phi * masked_grad + mu_i
                        self.ch.send_to_passive(masked_grad, index=index)
                        masked_theta_a = self.ch.recv_from_passive(index=index)
                        masked_theta_a = (masked_theta_a + mu_i) /_phi
                        phi_i = np.random.normal(0, 1)
                        while phi_i == 0:
                            phi_i = np.random.normal(0, 1)
                        masked_theta_a *= phi_i
                        self.ch.send_to_passive(masked_theta_a, index=index)
                        return phi_i
                    _phi_list = [phi_list[idx][e*len(batch_idx)+i] for idx in range(self.max_client_num)]
                    new_phi_list = Parallel(n_jobs=self.max_client_num)(
                        delayed(_update_grad)(_phi_list[idx], idx) for idx in range(self.max_client_num)
                    )
                    for idx in range(self.max_client_num):
                        phi_list[idx].append(new_phi_list[idx])
                    
                s2 = time()
                runtime_batch = s2 - s1
                self.logger.saveMsg(f"time of training a batch: {runtime_batch:.4f} seconds")
        self.theta = theta_b
        end_time = time()
        run_time = end_time - start_time
        self.logger.saveMsg(f"Training is over, the total time is {run_time:.4f} seconds.")

        self.logger.saveMsg(f"Start Validation")

    def predict(self, X):
        z_b = np.dot(X, self.theta)
        z_a = self.ch.recv_from_passive()
        if len(z_a) > 1:
            z = z_b
            for _z in z_a:
                z += _z
        else:
            z = z_a + z_b
        y_pred = self.sigmoid(z)
        return np.where(y_pred>=0.5, 1, 0)

class ISSVLRPassiveClient(BasePassiveClient):
    def __init__(self, config):
        super().__init__(config)                
        self.logger.saveMsg("Passive party start.")
        self.logger.saveMsg("Create Channel for Communication.")

        self.K_num = 10
        self.K_list = []

    def fit(self, X=None):
        if X is None:
            X, val_X = self.load_data()
        n = X.shape[0]
        m_a = X.shape[1]
        

        self.logger.saveMsg("Start training.")
        theta_a = np.random.normal(0, 1, m_a)
        for e in range(self.epochs):
            self.logger.saveMsg("Epoch: {}.".format(e))
            batch_idx = self.ch.recv_from_active()
            for i in tqdm(range(len(batch_idx))):
                idx_list = batch_idx[i]
                batch_X = X[idx_list].astype(float)
                z_a = np.dot(batch_X, theta_a)
                self.ch.send_to_active(z_a)
                masked_delta = self.ch.recv_from_active()
                masked_grad_S_a = np.dot(batch_X.T, masked_delta)
                K_i = np.random.normal(0, 1, (m_a, m_a))
                while np.linalg.det(K_i) == 0:
                    K_i = np.random.normal(0, 1, (m_a, m_a))
                K_i_inverse = np.linalg.inv(K_i)
                masked_grad_S_a = np.dot(K_i, masked_grad_S_a)
                S_i = np.sum(batch_X, axis=0)
                S_i /= self.batch_size
                masked_S_i = np.dot(K_i, S_i)
                self.ch.send_to_active([masked_grad_S_a, masked_S_i])
                masked_grad_a = self.ch.recv_from_active()
                masked_theta_a = np.dot(K_i, theta_a) -  masked_grad_a
                self.ch.send_to_active(masked_theta_a)
                masked_theta_a = self.ch.recv_from_active()
                theta_a = np.dot(K_i_inverse, masked_theta_a)
        self.theta = theta_a
        self.logger.saveMsg("Training is over.")



    def predict(self, X):
        z_a = np.dot(X, self.theta)
        self.ch.send_to_active(z_a)