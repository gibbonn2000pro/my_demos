import time



class Window(list):
    def __init__(self):
        super(Window, self).__init__()
        pass

    def put_data(self, timestamp, data):
        self.append((timestamp, data))
        pass


    def __tidy__(self, timestamp):
        while len(self):
            (t, _) = self[0]
            if timestamp - t > 60:
                self.pop(0)
            else:
                return
            pass
        pass

    def get_data(self, timestamp):
        raise Exception("not implemented")
        pass

    pass


class ValWindow(Window):
    def get_data(self, timestamp):
        self.__tidy__(timestamp)
        l = len(self)
        if l:
            return self[l - 1]
        else:
            return 0
        pass


class AccWindow(Window):
    def get_data(self, timestamp):
        self.__tidy__(timestamp)
        l = len(self)
        if l:
            return reduce(lambda s, (_, n): s + n, self, 0)
        else:
            return 0
        pass
        
def gen_timestamp():
    return int(time.time())
    pass


if __name__ == '__main__':
    am = AccWindow()
    am.put_data(gen_timestamp(), 1)
    am.put_data(gen_timestamp(), 2)
    am.put_data(gen_timestamp(), 3)

    print am
    print am.get_data(gen_timestamp())
    print am.get_data(gen_timestamp()+61)
    print am
