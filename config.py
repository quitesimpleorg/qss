import os
DBPATH=os.getenv("QSS_PATH")
if DBPATH == None or DBPATH == "":
    print("MIssing env var")
    exit(1)
