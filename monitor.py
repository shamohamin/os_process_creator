import matplotlib.pyplot as plt
import numpy as np
import os
import json


class Parser(object):
    EXECUTION_TIME = 'execution time'
    CONFIGURATION = 'configuration'
    MAIN_PROCESSES = 'MAIN PROCESSES'
    CHILD_PROCESSES = 'CHILD_PROCESSES'
    CREATED_PROCESS = 'process created'
    PROCESS_LEVEL_1 = 'number of process level 1'
    PROCESS_LEVEL_2 = 'number of process level 2'
    CHILD_NUMBER = "child process number"
    PARRENT_NUMBER = "parrent process number"
    PARRENT_ID = "parrent_id"

    def __init__(self, filepath, *args) -> None:
        super().__init__(*args)
        self.filepath = filepath
        self.check()
        self.setup()

    def setup(self):
        self.extractedOutput = {
            Parser.MAIN_PROCESSES: [],
            Parser.CREATED_PROCESS: []
        }

    def createProcessInfo(self, time, process_num, parrent_num, process_id, parrent_id) -> dict:
        return {
            Parser.EXECUTION_TIME: time,
            'id': process_id,
            Parser.PARRENT_ID: parrent_id,
            Parser.CHILD_NUMBER: process_num,
            Parser.PARRENT_NUMBER: parrent_num
        }

    def createConfigure(self, time, process_count, child_process_count) -> dict:
        return {
            Parser.EXECUTION_TIME: time,
            Parser.CONFIGURATION: {
                Parser.PROCESS_LEVEL_1: process_count,
                Parser.PROCESS_LEVEL_2: child_process_count
            }
        }

    def check(self):
        if not os.path.exists(self.filepath):
            raise Exception("File Not Found!!!")

    def readFile(self):
        file = open(self.filepath, "r")
        self.output = json.loads(file.read())
        file.close()

    def extractExecutionTime(self):
        for item in self.output:
            self.extractedOutput.update({
                Parser.MAIN_PROCESSES:
                    [*self.extractedOutput.get(self.MAIN_PROCESSES),
                        self.createConfigure(
                            item[Parser.EXECUTION_TIME],
                            item[Parser.CONFIGURATION][0][Parser.PROCESS_LEVEL_1],
                            item[Parser.CONFIGURATION][0][Parser.PROCESS_LEVEL_2]
                    )]
            })

            for child in item[Parser.CREATED_PROCESS]:
                self.extractedOutput.update({
                    Parser.CREATED_PROCESS: [*self.extractedOutput.get(Parser.CREATED_PROCESS),
                                             self.createProcessInfo(
                                            child[Parser.EXECUTION_TIME],
                                            child[Parser.CHILD_NUMBER],
                                            child[Parser.PARRENT_NUMBER],
                                            child['id'],
                                            child[Parser.PARRENT_ID])]
                })

    def execution(self):
        try:
            self.readFile()
            self.extractExecutionTime()
        except Exception as ex:
            print(ex.args[0])


class Monitor:
    def __init__(self):
        self.path = os.path.join(os.path.abspath(
            os.path.dirname(__file__)), 'output.json')
        self.parser = Parser(self.path)
        self.setup()

    def setup(self):
        self.parser.execution()
        self.mainProcess = self.parser.extractedOutput[self.parser.MAIN_PROCESSES]
        self.childProcess = self.parser.extractedOutput[self.parser.CREATED_PROCESS]

    def makeViewData(self):
        self.Y_DATA, self.X_DATA = [], []

        for item in self.mainProcess:
            self.Y_DATA.append(float(item[self.parser.EXECUTION_TIME]))
            self.X_DATA.append(str('first_level' +
                                   str(item[self.parser.CONFIGURATION][self.parser.PROCESS_LEVEL_1]) +
                                   '\n' + 'second level' +
                                   str(item[self.parser.CONFIGURATION][self.parser.PROCESS_LEVEL_2])))

        self.Y_DATA = np.array(self.Y_DATA)

    def draw(self):
        plt.figure(figsize=(9, 7.5))
        indexs = np.arange(start=0, stop=len(self.Y_DATA));
        plt.plot(self.Y_DATA, 'r-')
        for i, item in enumerate(self.Y_DATA):
            plt.plot([-1, i], [item, item], 'k--', linewidth=0.5)

        plt.scatter(x=indexs,
                    y=self.Y_DATA, s=80, marker='o', c='k', zorder=10, alpha=.9)
        plt.xticks(indexs, self.X_DATA)
        ticks = []
        for item in self.Y_DATA:
            ticks.append(str(item) + ' ms')
        plt.yticks(self.Y_DATA, ticks)
        plt.xlabel('configuration')
        plt.bar(indexs, self.Y_DATA, color=(0.1,0.5,0.5,1), width=.2);
        
        plt.axis([-1, len(indexs), np.min(self.Y_DATA) - 0.5, np.max(self.Y_DATA) + 1])
        plt.title(r'$Performance$')
        plt.ylabel(r'$EXECUTION$ $TIME$')
        plt.show()

    def execute(self):
        self.makeViewData()
        self.draw()


if __name__ == '__main__':
    Monitor().execute()
