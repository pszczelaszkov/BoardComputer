'''
WARNING!
This Script alters files so it needs to run on copy of source files!!

Keywords:
TESTUSE <code>
TESTSTATICVAR(static type varname)
TESTADDPREFIX(anything)

It makes 4 operations:
    1. Elevate names:
        -Replace TESTADDPREFIX(name) with "DIRFILENAME_" prefix,
        -Moves TESTUSE marked definitions from source file to header.
        -Creates setters/getters for variables in source file.
    2. Searches files for TESTUSE keyword(i.e TESTUSE int variable_name).
    3. Creates definitions headers for test usage.
    4. Wraps main file with cffi for ease of importing.

If object have static qualifier it will be elevated to extern.
TESTPREFIX typname will evaluate into PATHWITHOUTSLASHES_typename,
also it will replace further occurences in file.

For structs/unions keyword should be used in definitions as size is needed.
For the rest it should work in declarations.
'''
import sys
import os
import re
from collections import Counter
from itertools import chain
from dataclasses import dataclass
from cffi import FFI
from cffi import recompiler


class WrongFileExtension(Exception):
    pass


class ParsedData:

    def __init__(self):
        self.macrodefinitions = set()
        self.variables = []
        self.functions = []
        self.enumerations = []
        self.structs = []
        self.unions = []
        self.functiontypedefs = []

    def extend(self, data):
        self.macrodefinitions.update(data.macrodefinitions)
        self.variables.extend(data.variables)
        self.functions.extend(data.functions)
        self.enumerations.extend(data.enumerations)
        self.structs.extend(data.structs)
        self.unions.extend(data.unions)
        self.functiontypedefs.extend(data.functiontypedefs)


def split_path(path: str) -> str:
    return re.split(r"[\/\\.]", path)


def addprefixes(path: str, testdirectorypath: str) -> list:
    relativepath = path.replace(testdirectorypath, '').upper()
    prefix = ''.join(split_path(relativepath)[0:-1])
    names = []
    # For now there is no sense to scan .h
    if path.endswith('system.c'):
        brackets = Counter()
        with open(path, mode='r') as file:
            updatedlines = []
            for line in file.readlines():
                brackets.update([
                    character for character in line
                    if character == '{' or character == '}'
                    ])
                if brackets['{'] == brackets['}']:
                    if (match := re.search(r"TESTADDPREFIX\(\w*\)", line)):
                        matchedstring = match.group(0)
                        name = matchedstring.split(sep='(')[1][0:-1]
                        names.append(name)
                        line = line.replace(matchedstring, name)
                for name in names:
                    line = re.sub(
                        fr"{name}[^\W]*", f"{prefix}_{name}", line
                        )
                updatedlines.append(line)

            with open(path, mode='w') as file:
                file.writelines(updatedlines)
    return [f"{prefix}_{name}" for name in names]


def elevate_definitions(path: str):
    '''
    Scan .c source files for TESTUSE macro.
    If found, moves definitions into .h counterpart of this file.
    '''

    if not path.endswith(".c"):
        raise WrongFileExtension

    updatedsource = ''
    with open(path, mode='r') as source:
        with open(path.replace('.c', '.h'), mode='a') as header:
            header.write("\n"*2)
            updatedsource = source.read()
            data = scan_for_definitions(path)
            for datatype, definitions in data.__dict__.items():
                if datatype != "macrodefinitions":
                    for definition in definitions:
                        updatedsource = updatedsource.replace(
                            f"TESTUSE {definition}", '')
                        header.write(f"TESTUSE {definition}")

    with open(path, mode='w') as source:
        source.write(updatedsource)


def scan_for_definitions(path: str) -> ParsedData:
    '''
    Scans file under given path for TESTUSE macro.
    Returns copied definitions with stripped TESTUSE.
    '''
    macrodefinition = re.compile(r"#define \w* [\w* .\/]*\n")
    function = re.compile(r"(TESTUSE \w* ?\w+\** \w*\([\w,* &]*\))[\n;{]")
    variable = re.compile(r"(TESTUSE ?\w* ?\w* \w+\** [^()\n=;]+)([ =\w]*);")
    enumeration = re.compile(r"TESTUSE \w* ?enum \w+")
    struct = re.compile(r"TESTUSE \w* ?struct \w+")
    union = re.compile(r"TESTUSE \w* ?union \w+")
    functiontypedef = re.compile(r"TESTUSE typedef \w* \([*\w]*\)\([*\w]*\)")
    data = ParsedData()

    def get_the_objectbody(iterator, head):
        class ObjectBodyNotFound(Exception):
            pass

        iterator = chain(iter([head]), iterator)
        body = []
        match = False
        bracketcounter = 0
        for line in iterator:
            if line.find('{') > -1:
                bracketcounter = bracketcounter + 1
            elif line.find('}') > -1:
                bracketcounter = bracketcounter - 1
                if bracketcounter == 0:
                    regmatch = re.search(r"}\w*;", line)
                    line = line[:regmatch.end()]
                    match = True
            body.append(line)
            if match:
                return body

        raise ObjectBodyNotFound

    with open(path) as file:
        content = file.readlines()
        iterator = iter(content)
        for line in iterator:
            if ((definitionmatch := macrodefinition.search(line)) is not None):
                macro = definitionmatch.group(0).split()[1]
                data.macrodefinitions.update([macro])
            elif (variablematch := variable.search(line)) is not None:
                data.variables.append(variablematch.groups()[0].replace(
                    "TESTUSE ", '') + ";")
            elif (functionmatch := function.search(line)) is not None:
                data.functions.append(functionmatch.groups()[0].replace(
                    "TESTUSE ", '') + ";")
            elif enumeration.search(line) is not None:
                definition = []
                definition.extend(get_the_objectbody(
                    iterator, line.replace("TESTUSE ", '')))
                data.enumerations.append(''.join(definition))
            elif struct.search(line) is not None:
                definition = []
                definition.extend(get_the_objectbody(
                    iterator, line.replace("TESTUSE ", '')))
                data.structs.append(''.join(definition))
            elif union.search(line) is not None:
                definition = []
                definition.extend(get_the_objectbody(
                    iterator, line.replace("TESTUSE ", '')))
                data.unions.append(''.join(definition))
            elif (fnctypedefmatch := functiontypedef.search(line)) is not None:
                data.functiontypedefs.append(fnctypedefmatch.group(0).replace(
                    "TESTUSE ", '') + ";")

        return data


def elevate_staticvars(path: str, testdirectorypath: str):
    '''
    Scan .c source files for TESTSTATICVAR(static type varname) macro.
    If found, creates setter/getter in form of "DIRFILENAME_get/set_varname()".
    Which is elevated to .h counterpart of this file.
    '''
    if not path.endswith(".c"):
        raise WrongFileExtension

    relativepath = path.replace(testdirectorypath, '').upper()
    prefix = ''.join(split_path(relativepath)[0:-1])

    pattern = re.compile(r"TESTSTATICVAR\(static [\w*]* .*\)")
    staticvars = []
    with open(path) as source:
        with open(path.replace('.c', '.h'), mode='a') as header:
            header.write("\n"*2)
            header.write(f"#ifndef {prefix}_TESTGUARD\n")
            header.write(f"#define {prefix}_TESTGUARD\n")
            content = source.readlines()
            for line in content:
                if (match := pattern.search(line)):
                    staticvar = match.group(0)[14:-1].split()[1:]
                    declarations = [
                        f"TESTUSE {staticvar[0]} {prefix}_get{staticvar[1]}();\n",
                        (f"TESTUSE void {prefix}_set{staticvar[1]}" +
                            f"({staticvar[0]} value);\n")
                    ]
                    header.writelines(declarations)
                    staticvars.append(staticvar)
            header.write(f"#endif\n")

    with open(path, mode='a') as source:
        source.write("\n"*2)
        for staticvar in staticvars:
            setter = [
                f"void {prefix}_set{staticvar[1]}({staticvar[0]} value)",
                f"{{{staticvar[1]} = value;}}\n"
            ]
            getter = [
                f"{staticvar[0]} {prefix}_get{staticvar[1]}()"
                f"{{return {staticvar[1]};}}\n"
            ]
            source.writelines(setter + getter)


def get_files(dirpath):
    for entry in os.scandir(dirpath):
        if entry.is_file() and (entry.name.endswith(".c") or
                                entry.name.endswith(".h")):
            yield entry.path
        elif entry.is_dir():
            yield from get_files(entry.path)


def elevate(testdirectorypath: str):
    for filepath in get_files(testdirectorypath):
        if filepath.endswith('.c'):
            addprefixes(filepath, testdirectorypath)
            elevate_definitions(filepath)
            elevate_staticvars(filepath, testdirectorypath)


def scan_files(testdirectorypath):
    parsed = ParsedData()
    for filepath in get_files(testdirectorypath):
        if(filepath.endswith('.h')):
            parsed.extend(scan_for_definitions(filepath))
    return parsed


def create_definitions(path: str, parsed: ParsedData):
    # Creates Definitions.h
    definitionspath = path + "/definitions.h"
    if os.path.exists(definitionspath):
        os.remove(definitionspath)
    with open(definitionspath, "x") as definitions:
        sortedmacrodefinitions = list(parsed.macrodefinitions)
        sortedmacrodefinitions.sort()
        definitions.writelines([
            f"#define {macrodefinition} ...\n"
            for macrodefinition in sortedmacrodefinitions
            ])
        definitions.writelines(
            "%s\n" % enumeration for enumeration in parsed.enumerations)
        definitions.writelines(
            "%s\n" % line for line in parsed.functiontypedefs)
        structdeclarations = []
        uniondeclarations = []

        for structdefinition in parsed.structs:
            splittedstruct = re.split(r"[{}]", structdefinition)
            firstline = re.match(r"[\w ]*", splittedstruct[0]).group(0)
            istypedef = firstline.find("typedef") != -1
            if istypedef:
                lastline = re.match(r"\w*", splittedstruct[-1]).group(0)
                declaration = f"{firstline} {lastline}"
            else:
                declaration = firstline
            structdeclarations.append(f"{declaration};\n")
        for uniondefinition in parsed.unions:
            splittedunion = re.split(r"[{}]", uniondefinition)
            firstline = re.match(r"[\w ]*", splittedunion[0]).group(0)
            istypedef = firstline.find("typedef") != -1
            if istypedef:
                lastline = re.match(r"\w*", splittedunion[-1]).group(0)
                declaration = f"{firstline} {lastline}"
            else:
                declaration = firstline
            uniondeclarations.append(f"{declaration};\n")
        definitions.writelines(structdeclarations)
        definitions.writelines(uniondeclarations)
        definitions.writelines("%s\n" % struct for struct in parsed.structs)
        definitions.writelines("%s\n" % union for union in parsed.unions)
        for i in range(len(parsed.variables)):
            if parsed.variables[i].find("extern") == -1:
                parsed.variables[i] = f"extern {parsed.variables[i]}"
        definitions.writelines("%s\n" % line for line in parsed.variables)
        definitions.writelines("%s\n" % line for line in parsed.functions)


def wrapmain(testdirectory, generateddirectory):
    filepath = f'{testdirectory}/main.c'
    definitionspath = f"{generateddirectory}/definitions.h"
    with open(filepath) as source:
        with open(definitionspath) as definitions:
            ffibuilder = FFI()
            ffibuilder.cdef(definitions.read())
            recompiler.make_c_source(
                ffibuilder, "bin.testmodule", source.read(), filepath
                )


def main():
    testdirectorypath = sys.argv[1]
    generatedfilesdirectory = sys.argv[2]

    elevate(testdirectorypath)
    parsed = scan_files(testdirectorypath)
    create_definitions(generatedfilesdirectory, parsed)
    wrapmain(testdirectorypath, generatedfilesdirectory)


if __name__ == "__main__":
    main()
