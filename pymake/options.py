import os
import platform
from optparse import OptionParser


def get_command_line_options():
    parser = OptionParser()
    parser.add_option(
        "-m", "--math", dest="math_mode", help="Math mode to use", default="meth"
    )
    parser.add_option(
        "-t", "--target", dest="target", help="Target to build", default="it"
    )
    parser.add_option(
        "-h", "--help", dest="target", help="Display help message", default=False
    )
    parser.add_option(
        "-x",
        "--optimize",
        dest="optimize_level",
        help="Optimization level",
        default="3",
    )
    parser.add_option(
        "-d",
        "--debug-release",
        dest="debug_release",
        help="Enable debug build in release",
        default=False,
    )
    parser.add_option(
        "-g",
        "--gl-loader",
        dest="gl_loader",
        help="Which OpenGL loader to use",
        default="glad",
    )
    parser.add_option(
        "-o",
        "--os",
        dest="os",
        help="Target operating system",
        default=platform.system().lower(),
    )
    parser.add_option(
        "-a",
        "--arch",
        dest="arch",
        help="Target architecture",
        default=platform.machine().lower(),
    )
    parser.add_option(
        "-c",
        "--compiler",
        dest="compiler",
        help="Compiler to use",
        default="c++",
    )
    return parser.parse_args()
