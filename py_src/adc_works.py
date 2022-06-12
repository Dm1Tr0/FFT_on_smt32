import serial as s
import time
import matplotlib.pyplot as plt

class Stm_ser():
    def __init__(self, path_to_usb_dev):
        self.serial = s.Serial(path_to_usb_dev)
        print(self.serial.name)
        self.commands = {'start': '1', 'td_read': '2', 'dft_read': '3', 'fft_read': "4" }
        self.samples_per_round = 25
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

        print(f"query: {query}")
        self.serial.write(bytes(query, encoding='utf8'))
        return 0
    
    def get_smpl_cnt(self):
        return self.samples_cnt


    def read_data(self):
        if not self.rounds:
            print('there were no request for recording, nothing ro reade')
            return -1
        bin_output = bytes('', encoding='utf8')
        output = []
        for i in range(0,self.rounds):
            self.write('td_read')
            time.sleep(1)
            bin_output += self.serial.read_all()
            print(f"read from device cnt: {len(bin_output)}")
        
        for b in bin_output:
            print(format(b,"b"))

        for i in range(0,len(bin_output), 2):
            print(f"the first argumen:, the second argument: {bin_output[i + 1]}, the second argument to the pow: {bin_output[i + 1] * 2**8}")
            output.append(int(bin_output[i] + bin_output[i + 1] * 2**8))

        return output
            
            


stm = Stm_ser('/dev/ttyACM1')
stm.write("start 50")
time.sleep(11)
# stm.write("td_read")
# time.sleep(10)
# stm.write("td_read")
# time.sleep(10)
# time.sleep("td_read")

data = stm.read_data()
print(data)

figure, axis = plt.subplots(1,1)

axis.plot(data)

axis.set_title("data")

plt.show()