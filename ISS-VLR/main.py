from protocols.ISSVLR import ISSVLRActiveClient, ISSVLRPassiveClient
import argparse
import json

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--type", type=int, help="client type, 0 denotes passive party, 1 denotes active party.")
    parser.add_argument("-c", "--config", type=str, help="config file path.")

    args = parser.parse_args()
    with open(args.config) as f:
        config = f.read()
        config = json.loads(config)

    if args.type == 0:
        client = ISSVLRPassiveClient(config)
    else:
        client = ISSVLRActiveClient(config)

    
    client.fit()