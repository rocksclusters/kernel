class RocksEnv(object):
    def __init__(self):
        self.cmdline = open('/proc/cmdline','r').read().strip()
        p=map(lambda x: x.split("="),self.cmdline.split())
        self.cmdargs=dict(map(lambda x: x if len(x) == 2 else [x[0],x[0]],p))
    @property
    def clientInstall(self):
        """ read /proc/cmdline and determine if a client install """ 
        try:
            return self.cmdargs['rocks'] == 'client'
        except:
            return False 

    @property
    def trackerPath(self):
        """ read /proc/cmdline and get the tracker.path """
        try:
            return self.cmdargs['tracker.path']
        except:
            return None 

    @property
    def central(self):
        """ read /proc/cmdline and get the central"""
        try:
            return self.cmdargs['central']
        except:
            return None 
    
