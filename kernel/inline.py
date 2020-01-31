import subprocess
import re
from sys import argv

types = ['int', 'char', 'void']
include = re.compile('#include ".*"')

headers = []
class Header():
    def __init__(self, h):
        h_lst = h.strip().split('/')
        self.h = h_lst[-1]
        self.h_dir = '/'.join(h_lst[:-1])

    def __str__(self):
        return self.h_dir + '/' + self.h 

    def __eq__(self, other):
        if isinstance(other, str):
            return other == self.h
        if isinstance(other, Header):
            return other.h == self.h
        return False
    
    def write_header(self, outfile):
        f = open(str(self), 'r')
        outfile.write('\n// ----- START {0} -----\n'.format(self.h))
        for line in f:
            #print(line, end='')
            if (include.match(line)):
                outfile.write('// '+ line)
            else:
                outfile.write(line)
        outfile.write('\n// ----- END   {0} -----\n'.format(self.h))


modules = []
class Module():
    def __init__(self, c, h, f):
        self.modname = c[:-2]
        self.c = c
        self.h = h
        self.functions = f
    
    def __str__(self):
        return "Module: {0} {1}".format(self.h, self.c)

    def __eq__(self, other):
        if isinstance(other, str):
            return other == self.modname

    def write_header(self, outfile):
        f = open(self.h, 'r')
        outfile.write('\n// ----- START {0} -----\n'.format(self.h))
        for line in f:
            if defined_function(line):
                outfile.write("// " + line)
            else:
                if (include.match(line)):
                    outfile.write('// '+ line)
                else:
                    outfile.write(line)
        outfile.write('\n// ----- END   {0} -----\n'.format(self.h))

    def write_source(self, outfile):
        f = open(self.c, 'r')
        outfile.write('\n// ----- START {0} -----\n'.format(self.c))
        for line in f:
            if defined_function(line):
                outfile.write('__attribute__((always_inline))\n')
                outfile.write('static inline ' + line)
            else:
                if (include.match(line)):
                    outfile.write('// '+ line)
                else:
                    outfile.write(line)
        outfile.write('\n// ----- END   {0} -----\n'.format(self.c))


exclude = ['main', 'panic', 'print_lr']
functions = []
class Function():
    def __init__(self, tag):
        tag_list = tag.strip().split()
        self.name = tag_list[0]
        self.line = tag_list[2]
        self.file = tag_list[3]
        if ('{' in tag_list):
            tag_list.remove('{')
        self.defn = ' '.join(tag_list[4:])
        if (self.defn.strip()[-1] == ';'): self.defn = self.defn[:-1].strip() 
    def __str__(self):
        return self.defn + ';'

    def __eq__(self, other):
        if self.name in exclude:
            return False
        if isinstance(other, str):
            return (other == self.name) or (other == self.defn)
        return False

    def write_function(self, outfile):
        if self.name in exclude:
            outfile.write(str(self) + '\n')
        else:
            outfile.write('static inline ' + str(self) + '\n')


def output_headers(outfile, header):
    infile = open(header.__str__(), 'r')
    for line in infile.readlines():
        outfile.write(line)


def defined_function(l):
    line = str(l).strip().split()
    line_len = len(line)
    for i in range(len(line)):
        if line[i] in types:
            if (i+1 == line_len):
                return False
            fname = line[i+1].split('(')[0].strip('*')
            if fname in functions:
                return True
    return False


def get_structs(files):
    ctags = ('ctags -x --c-kinds=s ' + ' '.join(files)).strip().split()
    result = subprocess.Popen(ctags, stdout=subprocess.PIPE).stdout.read().decode('utf-8').strip().split('\n')
    for struct in result:
        types.append(struct.strip().split()[0])


def get_typedefs(files):
    ctags = ('ctags -x --c-kinds=t ' + ' '.join(files)).strip().split()
    result = subprocess.Popen(ctags, stdout=subprocess.PIPE).stdout.read().decode('utf-8').strip().split('\n')
    for typedef in result:
        types.append(typedef.strip().split()[0])


def get_functions(files):
    ctags = ('ctags -x --c-kinds=f ' + ' '.join(files)).strip().split()
    result = subprocess.Popen(ctags, stdout=subprocess.PIPE).stdout.read().decode('utf-8').strip().split('\n')
    for function in result:
        functions.append(Function(function))


def main(srcdir, builddir):
    # Recursively find all files in the srcdir
    find_files = 'find {0} -type f'.format(srcdir).split()
    files = subprocess.Popen(find_files, stdout=subprocess.PIPE).stdout.read().decode('utf-8').strip().split('\n')

    # Filter out .c and .h files into their own lists
    files = list(filter(lambda x: (x[-2] == '.' and (x[-1] == 'c' or x[-1] == 'h')), files))
    c_files = list(filter(lambda x: x[-1] == 'c', files))
    h_files = list(filter(lambda x: x[-1] == 'h', files))

    get_structs(files)
    get_typedefs(files)
    get_functions(c_files)


    # Put all the (.c,.h) file pairs into a structure
    # Remove all .h files with a corresponding .c file from the list
    # The remaining files are header-only modules
    # Assumes matching .h and .c are at least in the same directory
    main = ''
    for c in c_files:
        h = c[:-1]+'h'
        if h in h_files:
            f = list(filter(lambda x: x.file == c, functions))
            modules.append(Module(c,h,f))
            h_files.remove(h)
        else:
            main = Module(c, None, None)

    # Add header-only files into their own structures
    for h in h_files:
        headers.append(Header(h))

    outfile = open(builddir + '/inline-all.c', 'w')

    if 'constants.h' in headers:
        for h in headers:
            if h == 'constants.h':
                h.write_header(outfile)
                headers.remove(h)
                break

    for h in headers:
        h.write_header(outfile)

    for m in modules:
        m.write_header(outfile)

    outfile.write('\n// ----- START function definitions -----\n')
    for f in functions:
        f.write_function(outfile)
    outfile.write('\n// ----- END   function definitions -----\n')

    for m in modules:
        m.write_source(outfile)

    main.write_source(outfile)

#    ctags = 'ctags -x --c-kinds=f `find {0} -name \'*.c\'`'.format(srcdir).split()
#    functions = subprocess.Popen(ctags, stdout=subprocess.PIPE).stdout.read()
#    print(functions)


if __name__ == '__main__':
    srcdir = '.'
    builddir = '../build'
    if len(argv) > 1:
        srcdir = argv[1]
    if len(argv) > 2:
        builddir = argv[2]
    main(srcdir, builddir)

