class RocksEnv(object):
    def __init__(self):
        pass
    @property
    def clientInstall(self):
        cmdline = open('/proc/cmdline','r').read().strip()
        p=map(lambda x: x.split("="),cmdline.split())
        cmdargs=dict(map(lambda x: x if len(x) == 2 else [x[0],x[0]],p))
        try:
            return cmdargs['rocks'] == 'client'
        except:
            return False 
