import logging
import time

class Logger:
    def __init__(self, path, verbose):
        self.verbose = verbose
        self.path = path
        self.logger = self.start()

    def start(self):
        logger = logging.getLogger()
        logger.setLevel(logging.INFO)
        
        name = time.strftime("%Y-%m-%d-%H:%M:%S", time.localtime())
        name += '.txt'
        handler = logging.FileHandler(self.path+name)
        handler.setLevel(logging.INFO)
        
        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
        handler.setFormatter(formatter)
        logger.addHandler(handler)

        if self.verbose:
            console = logging.StreamHandler()
            console.setLevel(logging.INFO)
            console.setFormatter(formatter)
            logger.addHandler(console)

        return logger
    
    def saveMsg(self, msg):
        msg = "="*10 + " " + msg
        self.logger.info(msg)