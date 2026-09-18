class Library:
    binary: str
    headers: str

    def __init__(self, binary: str, headers: str):
        self.binary = binary
        self.headers = headers

    @staticmethod
    def header_only(loc: str):
    	return Library("", loc)
