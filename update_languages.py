import glob
import os
import sys

def main():
    files = glob.glob("samples/flowers/views/*.cpp")
    langs = ["en_US", "de_DE", "fr_FR", "it_IT", "es_ES", "pt_BR", "ru_RU", "zh_CN", "ja_JP"]
    for l in langs:
        # run command to update the languages
        os.system(f"lupdate {' '.join(files)} -ts samples/flowers/languages/{l}.ts")

if __name__ == "__main__":
    main()