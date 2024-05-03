import os
import sys

class CPPWriter:

    __tabWidth = 4
    __maxWidth = 80

    class Block:
        def __init__(self, writer):
            self.__writer = writer

        def __enter__(self):
            self.__writer.incIndent()
            return self.__writer

        def __exit__(self, type, value, tb):
            self.__writer.decIndent()

    def __init__(self, filename):
        self.__file = open(filename, 'w', encoding='utf-8')
        self.__line = ''
        self.__indent = 0
        self.writeln('//')
        self.writeln('// This is a generated file, do not change it manually.')
        self.writeln('// Use the script "%s" instead.' % os.path.split(sys.modules['__main__'].__file__)[-1])
        self.writeln('//')
        self.writeln()

    def __enter__(self):
        return self

    def __exit__(self, type, value, tb):
        self.__flush()
        self.__file.close()

    def __flush(self):
        self.__line = self.__line.rstrip()
        if self.__line:
            if self.__indent:
                self.__file.write(' ' * self.__indent)
            self.__file.write(self.__line)
            self.__file.write('\n')
        self.__line = ''

    def indent(self):
        return self.Block(self)

    def incIndent(self):
        self.__flush()
        self.__indent += CPPWriter.__tabWidth

    def decIndent(self):
        assert(self.__indent >= CPPWriter.__tabWidth)
        self.__flush()
        self.__indent -= CPPWriter.__tabWidth

    def writeln(self, line = ''):
        self.__flush()
        line = line.rstrip()
        if line:
            if self.__indent:
                self.__file.write(' ' * self.__indent)
            self.__file.write(line)
        self.__file.write('\n')

    def writeWithWrap(self, elem):
        if self.__indent + len(elem) + len(self.__line) > CPPWriter.__maxWidth:
            self.__flush()
        self.__line += elem

