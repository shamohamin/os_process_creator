import enum
import matplotlib.pyplot as plt
from matplotlib.pyplot import axis
import numpy as np
import os
import json
import glob
import re
import time


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
    CHILD_EXECUTION_TIME = "waiting of childs"

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

    def findIndex(self, process_count, child_process_count) -> int:
        for index, item in enumerate(self.extractedOutput[self.MAIN_PROCESSES]):
            item = item[self.CONFIGURATION]
            if item[self.PROCESS_LEVEL_1] == process_count and \
                    item[self.PROCESS_LEVEL_2] == child_process_count:
                return index
        return -1

    def createConfigure(self, time, process_count, child_process_count, child_execution) -> dict or None:
        index = self.findIndex(process_count, child_process_count)
        if index == -1:
            return {
                Parser.CONFIGURATION: {
                    Parser.EXECUTION_TIME: [time],
                    Parser.PROCESS_LEVEL_1: process_count,
                    Parser.PROCESS_LEVEL_2: child_process_count,
                    Parser.CHILD_EXECUTION_TIME: [child_execution]
                }
            }
        else:
            self.extractedOutput[self.MAIN_PROCESSES][index][Parser.CONFIGURATION].update({
                Parser.EXECUTION_TIME: [time] if index == -1 else [
                    *self.extractedOutput[self.MAIN_PROCESSES][index][Parser.CONFIGURATION][Parser.EXECUTION_TIME],
                    time
                ],
                Parser.CHILD_EXECUTION_TIME: [child_execution] if index == -1 else [
                    *self.extractedOutput[self.MAIN_PROCESSES][index][Parser.CONFIGURATION][Parser.CHILD_EXECUTION_TIME],
                    child_execution
                ]
            })
            return None

    def check(self) -> None:
        if not os.path.exists(self.filepath):
            raise Exception("File Not Found!!!")

    def readFile(self) -> None:
        file = open(self.filepath, "r")
        self.output = json.loads(file.read())
        file.close()

    def extractExecutionTime(self) -> None:
        for item in self.output:
            createdConfure = self.createConfigure(
                item[Parser.EXECUTION_TIME],
                item[Parser.CONFIGURATION][0][Parser.PROCESS_LEVEL_1],
                item[Parser.CONFIGURATION][0][Parser.PROCESS_LEVEL_2],
                item[Parser.CHILD_EXECUTION_TIME]
            )
            self.extractedOutput.update({
                Parser.MAIN_PROCESSES:
                    [*self.extractedOutput.get(self.MAIN_PROCESSES)] if createdConfure is None else [
                        *self.extractedOutput.get(self.MAIN_PROCESSES), createdConfure]
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

    def execution(self) -> None:
        try:
            self.readFile()
            self.extractExecutionTime()
        except Exception as ex:
            print(ex.args[0])


class Monitor:
    def __init__(self, parser=None) -> None:
        if parser is None:
            self.path = os.path.join(os.path.abspath(
                os.path.dirname(__file__)), 'output.json')
            self.parser = Parser(self.path)
        else:
            self.parser = parser
        self.setup()

    def setup(self) -> None:
        self.parser.execution()
        self.mainProcess = self.parser.extractedOutput[self.parser.MAIN_PROCESSES]
        self.childProcess = self.parser.extractedOutput[self.parser.CREATED_PROCESS]

    def makeViewData(self) -> None:
        self.Y_DATA, self.X_DATA, self.Y_DATA_CHILD = [], [], []
        for item in (self.mainProcess):
            item = item[self.parser.CONFIGURATION]
            self.Y_DATA_CHILD.append(item[self.parser.CHILD_EXECUTION_TIME])
            self.Y_DATA.append(item[self.parser.EXECUTION_TIME])
            self.X_DATA.append(str('first_level' +
                                   str(item[self.parser.PROCESS_LEVEL_1]) +
                                   '\n' + 'second level' +
                                   str(item[self.parser.PROCESS_LEVEL_2])))

        self.Y_DATA = np.vstack([self.Y_DATA, self.Y_DATA_CHILD])
        self.Y_DATA = np.array(self.Y_DATA)
        self.Y_DATA = np.average(self.Y_DATA, axis=1)
        self.Y_DATA = np.round(self.Y_DATA, 4)
        self.Y_DATA = self.Y_DATA.reshape(-1, 1)

    def draw(self) -> None:
        fig, axes = plt.subplots(1, 2)
        fig.set_size_inches((15, 8))
        for i, ax in enumerate(axes):
            start_index, end_index = i * \
                len(self.mainProcess), (i+1) * len(self.mainProcess)
            print(self.Y_DATA[start_index:end_index].ravel())

            indexs = np.arange(start=0, stop=len(
                self.Y_DATA[start_index:end_index].ravel()))

            ax.plot(self.Y_DATA[start_index:end_index], 'r-')
            for i, item in enumerate(self.Y_DATA[start_index: end_index].ravel()):
                ax.plot([-1, i], [item, item], 'k--', linewidth=0.5)

            ax.scatter(x=indexs,
                       y=self.Y_DATA[start_index:end_index].ravel(),
                       s=80, marker='o', c='k', zorder=10, alpha=.9)

            ax.set_xticks(indexs)
            ax.set_xticklabels(self.X_DATA)
            ticks = []
            for item in self.Y_DATA[start_index:end_index].ravel():
                ticks.append(str(item) + ' ms')
            ax.set_yticks(self.Y_DATA[start_index:end_index].ravel())
            ax.set_yticklabels(ticks)
            ax.set_xlabel('configuration')
            ax.bar(indexs, self.Y_DATA[start_index:end_index].ravel(), color=(
                0.1, 0.5, 0.5, 1), width=.2)

            ax.axis([-1, len(indexs), np.min(self.Y_DATA[start_index:end_index].ravel() - 1) -
                     0.5, np.max(self.Y_DATA[start_index:end_index].ravel()) + 3])
            ax.set_title(r'$Performance(After$ ' +
                     str(RunCFile.NUM_OF_TRYS) + r' $try)$' + '')
            ax.set_ylabel(r'$EXECUTION$ $TIME$')

        plt.show()
        

    def execute(self) -> None:
        self.makeViewData()
        self.draw()


class RunCFile:
    NUM_OF_TRYS = 10

    def __init__(self, num_of_trys=10) -> None:
        self.NUM_OF_TRYS = num_of_trys
        self.makefile = os.path.abspath(os.path.dirname(__file__))
        self.outputPath = os.path.join(os.path.abspath(
            os.path.dirname(__file__)), 'output.json')
        self.parser = Parser(self.outputPath)
        self.monitor = Monitor(self.parser)
        self.checkMakfile()

    def checkMakfile(self) -> bool:
        founded = glob.glob(os.path.join(self.makefile, r"*"))
        for f in founded:
            if re.search(pattern=r'(?:M|m)akefile$', string=f):
                return True
        raise Exception("Makefile or makefile not Found.")

    def run(self) -> None:
        os.system(f"cd {self.makefile} && make run")

    def execute(self) -> None:
        def mainExecution():
            for _ in range(self.NUM_OF_TRYS):
                self.run()
                self.parser.execution()
                time.sleep(.1)
        mainExecution()
        self.monitor.execute()


if __name__ == '__main__':
    RunCFile().execute()
