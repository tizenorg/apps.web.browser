# Translation script
# Reads files recursive from current directory
# replaces all found strings with translation by
# key, which is read from en.po file.
# Strings which have no translation available in en.po
# are left unchanged.



import os

texts = { }

def translateLine(line):
    translated = False
    splited = line.split('"')
    if len(splited) > 1:
        for i in range(len(splited) / 2):
            strNo = i * 2 + 1       # odd numbers
            # string longer than 1, check if there is translation for string
            if len(splited[strNo]) > 1 and splited[strNo] in texts:
                splited[strNo] = '_("' + texts[splited[strNo]] + '")'
                translated = True
            else:
                splited[strNo] = '"' + splited[strNo] + '"'

    if translated:
        return "".join(splited)
    else:
        return ""

def takeStringBetweenQuotations(s):
    return s.split('"')[1::2][0]

def translateFile(f):
    foutData = []
    fileTranslated = False
    with open(f, 'r') as fin:
        for line in fin:
            lineTranslated = False
            if not line.lstrip(' ').startswith('BROWSER_LOG'):
                newLine = translateLine(line)
                if len(newLine) > 0:
                    fileTranslated = True
                    foutData.append(newLine)
                else:
                    foutData.append(line)
            else:
                foutData.append(line)
    # Write the file out again
    if fileTranslated:
        print f
        with open(f, 'w') as fout:
            fout.write("".join(foutData))

def readKeys():
    with open('res/locale/en.po','r') as lang:
        value = ''
        for line in lang:
            if (line.startswith('msgid')):
                value = takeStringBetweenQuotations(line)
            elif (line.startswith('msgstr')):
                key = takeStringBetweenQuotations(line)
                if not value: raise Exception('Unknown file format, msgid should be read before msgstr')
                texts[key] = value

def translateFilesRecursive():
    print 'Translated files:'
    # replace strings in all files
    for root, subFolders, files in os.walk('.'):
        #for directory in subFolders:
        for f in files:
            if f.endswith('.cpp'):
                translateFile(os.path.join(root, f))


readKeys()
translateFilesRecursive()
