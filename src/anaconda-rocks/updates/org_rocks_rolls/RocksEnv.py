class RocksEnv(object):
    def __init__(self):
        pass
    @property
    def clientInstall(self):
	""" read /proc/cmdline and determine if a client install """ 
        cmdline = open('/proc/cmdline','r').read().strip()
        p=map(lambda x: x.split("="),cmdline.split())
        cmdargs=dict(map(lambda x: x if len(x) == 2 else [x[0],x[0]],p))
        try:
            return cmdargs['rocks'] == 'client'
        except:
            return False 

    @property
    def trackerPath(self):
	""" read /proc/cmdline and get the tracker.path """
        cmdline = open('/proc/cmdline','r').read().strip()
        p=map(lambda x: x.split("="),cmdline.split())
        cmdargs=dict(map(lambda x: x if len(x) == 2 else [x[0],x[0]],p))
        try:
            return cmdargs['tracker.path']
        except:
            return None 
