from matplotlib.pyplot import axis
import pandas as pd
import numpy as np
from sklearn.preprocessing import OneHotEncoder, LabelEncoder, MinMaxScaler
from sklearn.model_selection import train_test_split
import random
import copy
import math
import gzip
import os

def bisector_list(tabulation, num):
    new_list = []
    if len(tabulation)>=num:
        remainder = len(tabulation)%num
        if remainder == 0:
            merchant = int(len(tabulation) / num)
            for i in range(1,num+1):
                if i == 1:
                    new_list.append(tabulation[:merchant])
                else:
                    new_list.append(tabulation[(i-1)*merchant:i*merchant])
            return new_list
        else:
            merchant = round(len(tabulation) / num)
            for i in range(1, num + 1):
                if i == num:
                    new_list.append(tabulation[(i - 1) * merchant: ])
                else:
                    new_list.append(tabulation[(i - 1) * merchant:i * merchant])
            return new_list
    else:
        for i in range(1, len(tabulation) + 1):
            tabulation_subset = []
            tabulation_subset.append(tabulation[i - 1])
            new_list.append(tabulation_subset)
        return new_list

class Dataset:
    def __init__(self, train_path, test_path=None, names=None):
        self.train_data = pd.read_csv(train_path, header=None, names=names, index_col=None)
        if test_path:
            self.test_data = pd.read_csv(test_path, header=None, names=names, index_col=None)
        else:
            self.test_data = None
        if names is not None:
            self.names = names
        else:
            names = self.train_data.iloc[0, :]
            self.train_data = self.train_data.drop([0])
            self.train_data = self.train_data.rename(columns=names)
            self.names = names.tolist()
        self.label = None
        self.label_name = None
        self.continuous_att = None
        self.discrete_att = None
        self.exist_uk_att = None

    def preprocess(self, party_num=3, seed=None, valid=False):
        if self.test_data is not None:
            _train_data = self.train_data
            _test_data = self.test_data
        else:
            _train_data = self.train_data
            _train_data, _test_data = train_test_split(_train_data, test_size=0.2, random_state=42)
        
        if valid:
            _train_data, _valid_data = train_test_split(_train_data, test_size=0.2, random_state=42)

        if self.exist_uk_att is not None:
                _train_data = self._drop_unknown_value(_train_data)
                _test_data = self._drop_unknown_value(_test_data)
                if valid:
                    _valid_data = self._drop_unknown_value(_valid_data)

        train_labels = self._label_encoding(_train_data)
        test_labels = self._label_encoding(_test_data)
        if valid:
            valid_labels = self._label_encoding(_valid_data)

        _train_data = _train_data.drop([self.label_name], axis=1)
        _test_data = _test_data.drop([self.label_name], axis=1)
        if valid:
            _valid_data = _valid_data.drop([self.label_name], axis=1)

        _names = copy.deepcopy(self.names)
        _label_name = copy.deepcopy(self.label_name)
        _names.remove(_label_name)
        col_num = round(len(_names) / party_num)

        train_process_parties_data = []
        test_process_parties_data = []
        if valid:
            valid_process_parties_data = []

        if seed is not None:
            random.seed(seed)
        random.shuffle(_names)

        _new_names = bisector_list(_names, party_num)

        _name_num_sum = 0
        for i in range(party_num):
            party_name = _new_names[i]
            _name_num_sum += len(party_name)
            train_party_data = _train_data[party_name]
            test_party_data = _test_data[party_name]
            if valid:
                valid_party_data = _valid_data[party_name]
            party_cont_name = []
            party_disc_name = []
            party_other_name = []
            for name in party_name:
                if name in self.continuous_att:
                    party_cont_name.append(name)
                elif name in self.discrete_att:
                    party_disc_name.append(name)
                else:
                    party_other_name.append(name)
            if valid:
                train_process_d, test_process_d, valid_process_d = self._features_engineer(train_party_data, test_party_data,
                                                                        party_cont_name, party_disc_name, party_other_name, valid_party_data)
            else:
                train_process_d, test_process_d = self._features_engineer(train_party_data, test_party_data,
                                                                        party_cont_name, party_disc_name, party_other_name)
            train_process_parties_data.append(train_process_d)
            test_process_parties_data.append(test_process_d)
            if valid:
                valid_process_parties_data.append(valid_process_d)
        #print(_name_num_sum)
        if valid:
            return train_process_parties_data, valid_process_parties_data, test_process_parties_data, train_labels, valid_labels, test_labels
        else:
            return train_process_parties_data, test_process_parties_data, train_labels, test_labels

    def _label_encoding(self, data):
        pass

    def _features_engineer(self, train_data, test_data, continuous_att, discrete_att, other_att=None, valid_data=None):
        one_hot_encoder = OneHotEncoder(categories='auto')
        min_max_scaler = MinMaxScaler()
        train_process_d = pd.DataFrame()
        test_process_d = pd.DataFrame()
        if valid_data is not None:
            valid_process_d = pd.DataFrame()
        for att in continuous_att:
            temp_train_data = train_data[[att]]
            temp_test_data = test_data[[att]]
            if valid_data is not None:
                temp_valid_data = valid_data[[att]]
                temp_data = pd.concat([temp_train_data, temp_test_data, temp_valid_data], axis=0)
            else:
                temp_data = pd.concat([temp_train_data, temp_test_data], axis=0)
            min_max_clf = min_max_scaler.fit(temp_data.astype('float'))

            temp_train_data = min_max_clf.transform(temp_train_data.astype('float'))
            temp_train_data = pd.DataFrame(temp_train_data, columns=[att])
            train_process_d = pd.concat([train_process_d, temp_train_data], axis=1)

            temp_test_data = min_max_clf.transform(temp_test_data.astype('float'))
            temp_test_data = pd.DataFrame(temp_test_data, columns=[att])
            test_process_d = pd.concat([test_process_d, temp_test_data], axis=1)

            if valid_data is not None:
                temp_valid_data = min_max_clf.transform(temp_valid_data.astype('float'))
                temp_valid_data = pd.DataFrame(temp_valid_data, columns=[att])
                valid_process_d = pd.concat([valid_process_d, temp_valid_data], axis=1)

        for att in discrete_att:
            temp_train_data = train_data[[att]]
            temp_test_data = test_data[[att]]
            if valid_data is not None:
                temp_valid_data = valid_data[[att]]
                temp_data = pd.concat([temp_train_data, temp_test_data, temp_valid_data], axis=0)
            else:
                temp_data = pd.concat([temp_train_data, temp_test_data], axis=0)
            one_hot_clf = one_hot_encoder.fit(temp_data)

            temp_train_data = one_hot_clf.transform(temp_train_data).toarray()
            temp_train_data = pd.DataFrame(temp_train_data, columns=[att] * len(temp_train_data[0]))
            train_process_d = pd.concat([train_process_d, temp_train_data], axis=1)

            temp_test_data = one_hot_clf.transform(temp_test_data).toarray()
            temp_test_data = pd.DataFrame(temp_test_data, columns=[att] * len(temp_test_data[0]))
            test_process_d = pd.concat([test_process_d, temp_test_data], axis=1)

            if valid_data is not None:
                temp_valid_data = one_hot_clf.transform(temp_valid_data).toarray()
                temp_valid_data = pd.DataFrame(temp_valid_data, columns=[att] * len(temp_valid_data[0]))
                valid_process_d = pd.concat([valid_process_d, temp_valid_data], axis=1)

        for att in other_att:
            temp_train_data = train_data[[att]].astype('float')
            temp_test_data = test_data[[att]].astype('float')

            temp_train_data = temp_train_data.values
            temp_test_data = temp_test_data.values
            
            temp_train_data = pd.DataFrame(temp_train_data, columns=[att])
            temp_test_data = pd.DataFrame(temp_test_data, columns=[att])

            train_process_d = pd.concat([train_process_d, temp_train_data], axis=1)
            test_process_d = pd.concat([test_process_d, temp_test_data], axis=1)

            if valid_data is not None:
                temp_valid_data = valid_data[[att]].astype('float')
                temp_valid_data = temp_valid_data.values
                temp_valid_data = pd.DataFrame(temp_valid_data, columns=[att])
                valid_process_d = pd.concat([valid_process_d, temp_valid_data], axis=1)
        if valid_data is not None:
            return train_process_d, test_process_d, valid_process_d
        else:
            return train_process_d, test_process_d

    def _drop_unknown_value(self, data):
        for n in self.exist_uk_att:
            data = data[~data[n].isin([' ?', '   ?', '?'])]
        return data


class Adult(Dataset):
    def __init__(self, train_path, test_path=None):
        names = [
            'age', 'workclass', 'fnlwgt', 'education', 'education-num', 'marital-status', 'occupation', 'relationship',
            'race', 'sex', 'capital-gain', 'capital-loss', 'hours-per-week', 'native-country', 'label'
        ]
        Dataset.__init__(self, train_path, test_path, names)
        self.continuous_att = [
            'age',
            'fnlwgt',
            'education-num',
            'capital-gain',
            'capital-loss',
            'hours-per-week'
        ]
        self.discrete_att = [
            'workclass',
            'education',
            'marital-status',
            'occupation',
            'relationship',
            'race',
            'sex',
            'native-country'
        ]
        self.label_name = 'label'
        self.exist_uk_att = ['occupation', 'workclass', 'native-country']

    def _label_encoding(self, data):
        labels = [0 if ' <=50K' in x else 1 for x in data[self.label_name].values.tolist()]
        return labels

class Bank(Dataset):
    def __init__(self, train_path, test_path=None):
        names = [
            'age', 'job', 'marital', 'education', 'default', 'balance', 'housing', 'loan',
            'contact', 'day', 'month', 'duration', 'campaign', 'pdays', 'previous', 'poutcome', 'y'
        ]
        Dataset.__init__(self, train_path, test_path, names)
        self.continuous_att = [
            'age',
            'balance',
            'day',
            'duration',
            'campaign',
            'pdays',
            'previous'
        ]
        self.discrete_att = [
            'job',
            'marital',
            'education',
            'default',
            'housing',
            'loan',
            'contact',
            'month',
            'poutcome'
        ]
        self.label_name = 'y'
        self.exist_uk_att = None

    def _label_encoding(self, data):
        labels = [0 if x == 'no' else 1 for x in data[self.label_name].values.tolist()]
        return labels

class BreastCancer(Dataset):
    def __init__(self, train_path, test_path=None):
        names = [
            'Class', 'age', 'menopause', 'tumor-size', 'inv-nodes', 'node-caps', 'deg-malig', 'breast',
            'breast-quad', 'irradiat'
        ]
        Dataset.__init__(self, train_path, test_path, names)
        self.continuous_att = [

        ]
        self.discrete_att = [
            'age',
            'menopause',
            'tumor-size',
            'inv-nodes',
            'node-caps',
            'deg-malig',
            'breast',
            'breast-quad',
            'irradiat'
        ]
        self.label_name = 'Class'
        self.exist_uk_att = ['node-caps', 'breast-quad']

    def _label_encoding(self, data):
        labels = [0 if x == 'no-recurrence-events' else 1 for x in data[self.label_name].values.tolist()]
        return labels

class Slice(Dataset):
    def __init__(self, train_path, test_path=None):
        Dataset.__init__(self, train_path, test_path)
        self.continuous_att = self.names
        self.discrete_att = []
        self.label_name = 'reference'
        self.exist_uk_att = None

    def _label_encoding(self, data):
        labels = data[self.label_name].astype('float').values.tolist()
        return labels

class Ad(Dataset):
    def __init__(self, train_path, test_path=None, names=None):
        names = [str(i) for i in range(1559)]
        super().__init__(train_path, test_path, names)
        self.continuous_att = [str(i) for i in range(3)]
        self.discrete_att = []
        self.label_name = '1558'
        self.exist_uk_att = names

    def _label_encoding(self, data):
        labels = [0 if 'nonad' in x else 1 for x in data[self.label_name].values.tolist()]
        return labels

class Census(Dataset):
    def __init__(self, train_path, test_path=None, names=None):
        names = [
            'age', 'class_of_work', 'detailed_industry_recode', 'detailed_occupation_recode', 'education',
            'wage_per_hour', 'enroll_in_edu_inst_last_wk', 'marital_stat', 'major_industry_code', 'major_occupation_code',
            'race', 'hispanic_origin', 'sex', 'member_of_a_labor_union', 'reason_for_unemployment',
            'full_or_part_time_employment_stat', 'capital_gains', 'capital_losses', 'dividends_from_stocks', 'tax_filer_stat', 'region_of_previous_residence',
            'state_of_previous_residence', 'detailed_household_and_family_stat', 'detailed_household_summary_in_household', 'instance_weight', 'migration_code-change_in_msa', 
            'migration_code-change_in_reg', 'migration_code-move_within_reg', 'live_in_this_house_1_year_ago', 'migration_prev_res_in_sunbelt', 'num_persons_worked_for_employer',
            'family_members_under_18', 'country_of_birth_father', 'country_of_birth_mother', 'country_of_birth_self', 'citizenship', 
            'own_business_or_self_employed', 'fill_inc_questionnaire_for_veterans_admin', 'veterans_benefits', 'weeks_worked_in_year', 'year',
            'label'
        ]
        super().__init__(train_path, test_path, names)
        self.train_data = self.train_data.drop(['instance_weight'], axis=1)
        self.names.remove('instance_weight')
        self.continuous_att = ['age', 'wage_per_hour', 'capital_gains', 'capital_losses', 'dividends_from_stocks', 'num_persons_worked_for_employer', 'weeks_worked_in_year']
        self.discrete_att = list(set(self.names).difference(set(self.continuous_att)))
        self.discrete_att.remove('label')
        self.label_name = 'label'
        self.exist_uk_att = None
    
    def _label_encoding(self, data):
        labels = [0 if '- 50000' in x else 1 for x in data[self.label_name].values.tolist()]
        return labels

class Chess(Dataset):
    def __init__(self, train_path, test_path=None, names=None):
        names = [str(i) for i in range(37)]
        super().__init__(train_path, test_path, names)
        self.continuous_att = []
        self.discrete_att = [str(i) for i in range(36)]
        self.label_name = '36'
        self.exist_uk_att = None
    
    def _label_encoding(self, data):
        labels = [0 if 'nowin' in x else 1 for x in data[self.label_name].values.tolist()]
        return labels

class CreditCard(Dataset):
    def __init__(self, train_path, test_path=None, names=None):
        super().__init__(train_path, test_path, names)
        self.train_data = self.train_data.drop([self.train_data.columns[0]], axis=1)
        self.train_data = self.train_data.drop([1])
        self.names.pop(0)
        self.label_name = 'Y'
        self.continuous_att = ['X1', 'X5', 'X12', 'X13', 'X14', 'X15', 'X16', 'X17', 'X18', 'X19', 'X20', 'X21', 'X22', 'X23']
        self.discrete_att = ['X2', 'X3', 'X4', 'X6', 'X7', 'X8', 'X9', 'X10', 'X11']
        self.exist_uk_att = None

    def _label_encoding(self, data):
        labels = [0 if '0' in x else 1 for x in data[self.label_name].values.tolist()]
        return labels

class Nursery(Dataset):
    def __init__(self, train_path, test_path=None, names=None):
        names = ['parents', 'has_nurs', 'form', 'children', 'housing', 'finance', 'social', 'health', 'label']
        super().__init__(train_path, test_path, names)
        self.label_name = 'label'
        self.continuous_att = []
        self.discrete_att = ['parents', 'has_nurs', 'form', 'children', 'housing', 'finance', 'social', 'health']
        self.exist_uk_att = None
    
    def _label_encoding(self, data):
        labels = [0 if 'not_recom' in x else 1 for x in data[self.label_name].values.tolist()]
        return labels

def load_mnist(data_folder):
    files = ['train-labels-idx1-ubyte.gz', 'train-images-idx3-ubyte.gz', 't10k-labels-idx1-ubyte.gz', 't10k-images-idx3-ubyte.gz']
    
    paths = []
    for fname in files:
        paths.append(os.path.join(data_folder, fname))
          
    with gzip.open(paths[0], 'rb') as lbpath:
        y_train = np.frombuffer(lbpath.read(), np.uint8, offset=8)
      
    with gzip.open(paths[1], 'rb') as imgpath:
        x_train = np.frombuffer(imgpath.read(), np.uint8, offset=16).reshape(len(y_train), 784)
       
    with gzip.open(paths[2], 'rb') as lbpath:
        y_test = np.frombuffer(lbpath.read(), np.uint8, offset=8)
        
    with gzip.open(paths[3], 'rb') as imgpath:
        x_test = np.frombuffer(imgpath.read(), np.uint8, offset=16).reshape(len(y_test), 784)
    
    x_train_gz = np.transpose(x_train)
    x_test_gz = np.transpose(x_test)
    x_train_gz_ = []
    x_test_gz_ = []
    for i in range(len(x_train_gz)):
        minn = np.min(x_train_gz[i])
        maxx = np.max(x_train_gz[i])
        dif = maxx - minn
        if dif == 0:
            dif = 1
        x_train_gz_.append((x_train_gz[i] - minn) / dif)
        x_test_gz_.append((x_test_gz[i] - minn) / dif)
    x_train = np.array(list(zip(*x_train_gz_)))
    x_test = np.array(list(zip(*x_test_gz_)))
        
    return x_train,y_train,x_test,y_test
    
def save_uci_data(ds_name, n_parties=3):
    if ds_name == 'adult':
        train_ds = Adult('/home/hzd/dataset/adult/adult.data',
                         '/home/hzd/dataset/adult/adult.test')
    elif ds_name == 'bank':
        train_ds = Bank('/home/hzd/dataset/bank/bank/bank-full.csv')
    elif ds_name == 'breast':
        train_ds = BreastCancer('/home/hzd/dataset/breast-cancer/breast-cancer.data')
    elif ds_name == 'slice':
        train_ds = Slice('/home/hzd/dataset/slice/slice_localization_data.csv')
    elif ds_name == 'ad':
        train_ds = Ad('/home/hzd/dataset/ad/ad.data')
    elif ds_name == 'census':
        train_ds = Census('/home/hzd/dataset/census/census-income.data', '/home/hzd/dataset/census/census-income.test')
    elif ds_name == 'chess':
        train_ds = Chess('/home/hzd/dataset/chess/kr-vs-kp.data')
    elif ds_name == 'credit':
        train_ds = CreditCard('/home/hzd/dataset/credit-card/default_of_credit_card_clients.csv')
    elif ds_name == 'nursery':
        train_ds = Nursery('/home/hzd/dataset/nursery/nursery.data')
    else:
        train_ds = None
        raise ValueError('dataset name is error.')
    
    train_X, test_X, train_y, test_y = train_ds.preprocess(party_num=n_parties, valid=False)
    train_y_np = np.array(train_y)
    test_y_np = np.array(test_y)
    
    for i in range(len(train_X)):
        train_x_np = np.array(train_X[i])
        test_x_np = np.array(test_X[i])
        print(train_x_np.shape)
        np.savetxt('./{}-party/{}_train_data_{}.csv'.format(n_parties, ds_name, i), train_x_np, delimiter=',')
        np.savetxt('./{}-party/{}_test_data_{}.csv'.format(n_parties, ds_name, i), test_x_np, delimiter=',')
    np.savetxt('./{}-party/{}_train_label.csv'.format(n_parties, ds_name), train_y_np, delimiter=',')
    np.savetxt('./{}-party/{}_test_label.csv'.format(n_parties, ds_name), test_y_np, delimiter=',')

def save_mnist(party_num=3):
    data_file = '/home/hzd/experiment/lr-benchmark/lr-accuarcy/data/MNIST'
    x_train,y_train,x_test,y_test = load_mnist(data_file)
    def _split_array_by_column(array, k):
        num_columns = array.shape[1]
        column_indices = np.array_split(np.arange(num_columns), k)
        result = []

        for indices in column_indices:
            result.append(array[:, indices])

        return result
    x_train = _split_array_by_column(x_train, party_num)
    for i, _x in enumerate(x_train):
        print(_x.shape)
        np.savetxt('./{}-party/mnist_train_data_{}.csv'.format(party_num, i), _x, delimiter=',')
    x_test = _split_array_by_column(x_test, party_num)
    for i, _x in enumerate(x_train):
        print(_x.shape)
        np.savetxt('./{}-party/mnist_test_data_{}.csv'.format(party_num, i), _x, delimiter=',')
    np.savetxt('./{}-party/mnist_train_label.csv'.format(party_num), y_train, delimiter=',')
    np.savetxt('./{}-party/mnist_test_label.csv'.format(party_num), y_test, delimiter=',')

def generate_syn_data(n, m):
    matrix = np.random.random((n, m))
    vector = np.random.random((n, 1))

    matrix_df = pd.DataFrame(matrix)  
    matrix_df.to_csv("./syn_features.csv", index=False)  

    vector_df = pd.DataFrame(vector)  
    vector_df.to_csv("./syn_labels.csv", index=False) 

def generate_sparse_data(rows, cols, density):
    num_nonzero_elements = int(rows * cols * density)

    dense_matrix = np.zeros((rows, cols))

    nonzero_indices = np.random.choice(rows * cols, num_nonzero_elements, replace=False)
    row_indices, col_indices = np.unravel_index(nonzero_indices, (rows, cols))
    data = np.random.rand(num_nonzero_elements)

    dense_matrix[row_indices, col_indices] = data


    matrix_df = pd.DataFrame(dense_matrix)  
    matrix_df.to_csv("./" + str(density) + "sparse_syn_features.csv", index=False)  

    vector = np.random.random((rows, 1))
    vector_df = pd.DataFrame(vector)  
    vector_df.to_csv("./" + str(density) + "sparse_syn_labels.csv", index=False) 

if __name__ == '__main__':
    # ds_name_ls = ['adult', 'bank', 'census', 'ad', 'credit']
    # # ds_name_ls = ['census']
    # for ds_name in ds_name_ls:
    #     print(ds_name)
    #     save_uci_data(ds_name, n_parties=1)
    # save_mnist()
    # generate_syn_data(64, 40)
    density_list = [0.5, 0.1, 0.01, 0.001]
    for density in density_list:
        generate_sparse_data(64, 5000, density)