'''
WARNING!
This Script alters files so it needs to run on copy of source files!!

It makes 3 operations:
    1. Searches files for TESTUSE keyword(i.e TESTUSE int variable_name).
    2. Creates definitions headers for test usage.
    3. Wraps source files with correct definitions.

If object have static qualifier it will be removed and name altered.
'''
import sys
import os
import re
from itertools import chain
from dataclasses import dataclass
from textwrap import wrap
from cffi import FFI
from cffi import recompiler


@dataclass
class ParsedData:
    fileEntries = []
    macrodefinitions = set()
    variables = []
    functions = []
    enumerations = []
    structs = []
    unions = []
    functiontypedefs = []


def scanFile(path):
    macrodefinition = re.compile(r"#define \w* [\w* .\/]*\n")
    function = re.compile(r"(TESTUSE \w* ?\w+\** \w*\([\w,* &]*\))[\n;{]")
    variable = re.compile(r"(TESTUSE ?\w* ?\w* \w+\** [^()\n=;]+)([ =\w]*);")
    enumeration = re.compile(r"TESTUSE \w* ?enum \w+")
    struct = re.compile(r"TESTUSE \w* ?struct \w+")
    union = re.compile(r"TESTUSE \w* ?union \w+")
    functiontypedef = re.compile(r"TESTUSE typedef \w* \([*\w]*\)\([*\w]*\)")
    variables = []
    functions = []
    definitions = []
    enumerations = []
    structs = []
    unions = []
    functiontypedefs = []
    isheader = path.endswith(".h")

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
            if ((definitionmatch := macrodefinition.search(line)) is not None
                    and isheader):
                macro = definitionmatch.group(0).split()[1]
                definitions.append(macro)
            elif (variablematch := variable.search(line)) is not None:
                variables.append(variablematch.groups()[0].replace(
                    "TESTUSE ", '') + ";")
            elif (functionmatch := function.search(line)) is not None:
                functions.append(functionmatch.groups()[0].replace(
                    "TESTUSE ", '') + ";")
            elif enumeration.search(line) is not None:
                definition = []
                definition.extend(get_the_objectbody(
                    iterator, line.replace("TESTUSE ", '')))
                enumerations.append(''.join(definition))
            elif struct.search(line) is not None:
                definition = []
                definition.extend(get_the_objectbody(
                    iterator, line.replace("TESTUSE ", '')))
                structs.append(''.join(definition))
            elif union.search(line) is not None:
                definition = []
                definition.extend(get_the_objectbody(
                    iterator, line.replace("TESTUSE ", '')))
                unions.append(''.join(definition))
            elif (fnctypedefmatch := functiontypedef.search(line)) is not None:
                functiontypedefs.append(fnctypedefmatch.group(0).replace(
                    "TESTUSE ", '') + ";")

    if variables or functions or definitions or enumerations:
        return {
            "variables": variables,
            "functions": functions,
            "definitions": definitions,
            "enumerations": enumerations,
            "structs": structs,
            "unions": unions,
            "functiontypedefs": functiontypedefs,
        }
    else:
        return None


def get_files(dirpath):
    for entry in os.scandir(dirpath):
        if entry.is_file() and (entry.name.endswith(".c") or
                                entry.name.endswith(".h")):
            yield entry.path
        elif entry.is_dir():
            yield from get_files(entry.path)


def createDefinitions(path: str, parsed: ParsedData):
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
    parsed = ParsedData()

    for filepath in get_files(testdirectorypath):
        fileEntry = {
            "path": filepath.replace(testdirectorypath, '').split(os.sep),
            "entries": scanFile(filepath)
        }
        if fileEntry["entries"]:
            parsed.fileEntries.append(fileEntry)
            entries = fileEntry["entries"]
            if entries["variables"]:
                parsed.variables.extend(entries["variables"])
            if entries["functions"]:
                parsed.functions.extend(entries["functions"])
            if entries["definitions"]:
                for macrodefinition in entries["definitions"]:
                    parsed.macrodefinitions.add(macrodefinition)
            if entries["enumerations"]:
                parsed.enumerations.extend(entries["enumerations"])
            if entries["structs"]:
                parsed.structs.extend(entries["structs"])
            if entries["unions"]:
                parsed.unions.extend(entries["unions"])
            if entries["functiontypedefs"]:
                parsed.functiontypedefs.extend(entries["functiontypedefs"])

    createDefinitions(generatedfilesdirectory, parsed)
    wrapmain(testdirectorypath, generatedfilesdirectory)


if __name__ == "__main__":
    main()
