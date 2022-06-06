import serial as s
import time
import matplotlib.pyplot as plt

class Stm_ser():
    def __init__(self, path_to_usb_dev):
        self.serial = s.Serial(path_to_usb_dev)
        print(self.serial.name)
        self.commands = {'start': '1', 'td_read': '2', 'dft_read': '3', 'fft_read': "4" }
        self.samples_per_round = 50
        self.samples_cnt = 0
        self.rounds = 0

    def write(self, cmd_string):
        cmd_list = cmd_string.split()
        if len(cmd_list) > 2 :
            print('invalid command')
            return -1
        query = ''
        if cmd_list[0] == 'start' and len(cmd_list) > 1:
            self.samples_cnt = int(cmd_list[1]) 
            self.rounds = self.samples_cnt // self.samples_per_round + int((self.samples_cnt - self.samples_cnt // self.samples_per_round) != 0) 
            print(self.rounds)
        
        for i in range(0,len(cmd_list)):
            if i == len(cmd_list) - 1 and len(cmd_list) != 1 :
                query += cmd_list[i]
            else :
                query += self.commands[cmd_list[i]] + ' '

        print(query)
        self.serial.write(bytes(query, encoding='utf8'))
        return 0
    
    def get_smpl_cnt(self):
        return self.samples_cnt


    def read_data(self):
        if not self.rounds:
            print('there were no request for recording, nothing ro reade')
            return -1
        output = bytes('', encoding='utf8')
        for i in range(0,self.rounds):
            self.write('td_read')
            
            output += self.serial.read_all()
            print(len(output))
        return (output)


stm = Stm_ser('/dev/ttyACM1')

stm.write('start 101')
time.sleep(0.5)
string = stm.read_data()
print(f"{string}, size: {len(string)}")


# figure, axis = plt.subplots(1,1)

# axis.plot(string)
# axis.set_title("clean")

# plt.show()