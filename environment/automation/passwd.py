class Passwd(object) :

    #
    # Description
    #   The class constructor
    #
    # Parameters
    #   _path - The path to "/etc/passwd" equivalent file
    #
    # Returns
    #    Nothing
    #
    def __init__(self, _path, _verbose = False) :
        self.path    = _path
        self.verbose = _verbose
        self.items   = []

        self.__parse()

    #
    # Description
    #   This function creates an item of passwd file
    #
    # Parameters
    #   username - The name of the user
    #   x        - The token indicating that the password is encrypted in /etc/shadow
    #   uid      - The user ID
    #   gid      - The group ID
    #   gecos    - The GECOS field containing user information (e.g. nickname)
    #   home     - The associated home directory
    #   shell    - The associated shell
    #
    # Returns
    #    A dictionary item
    #
    def __make_item(self, username, x, uid, gid, gecos, home, shell) :
        return {
            "username" : username,
            "x"        : x,
            "uid"      : uid,
            "gid"      : gid,
            "gecos"    : gecos,
            "home"     : home,
            "shell"    : shell
        }

    #
    # Description
    #   This function parse the given passwd file
    #
    # Parameters
    #   None
    #
    # Returns
    #    Nothing
    #
    def __parse(self) :

        with open(self.path, "r") as file :
            for line in file :
                tokens = line.split(':')

                if len(tokens) != 7 :
                    if self.verbose :
                        print("Invalid line : {}".format(line))
                        continue

                self.add_entry(tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], tokens[5], tokens[6].rstrip())

    #
    # Description
    #   This function adds an entry to the file
    #
    # Parameters
    #   username - The name of the user
    #   x        - The token indicating that the password is encrypted in /etc/shadow
    #   uid      - The user ID
    #   gid      - The group ID
    #   gecos    - The GECOS field containing user information (e.g. nickname)
    #   home     - The associated home directory
    #   shell    - The associated shell
    #
    # Returns
    #    Nothing
    #
    def add_entry(self, username, uid, gid, x="x", gecos=None, home=None, shell="/bin/bash") :

        if not gecos :
            gecos = username

        if not home :
            home = "/home/{}".format(username)

        item = self.__make_item(username, x, uid, gid, gecos, home, shell)

        if self.verbose :
            print("Adding item : {}".format(item))

        self.items.append(item)

    #
    # Description
    #   This function indicates if the user is found in the file
    #
    # Parameters
    #   username - The name of the user
    #
    # Returns
    #    True  - The user was found
    #    False - The user was not found
    #
    def is_user_in(self, username) :
        for item in self.items :
            if item["username"] == username :
                return True

        return False

    #
    # Description
    #   This function write the passwd to the given path
    #
    # Parameters
    #   path - The path where to write the file
    #
    # Returns
    #   Nothing
    #
    def write_to(self, path) :
        with open(path, "w") as file :
            self.write_in(file)

    #
    # Description
    #   This function write the passwd content to the given file
    #
    # Parameters
    #   file - The file object
    #
    # Returns
    #   Nothing
    #
    def write_in(self, file) :
        for item in self.items :
            file.write("{}:{}:{}:{}:{}:{}:{}\n".format(
                item["username"],
                item["x"],
                item["uid"],
                item["gid"],
                item["gecos"],
                item["home"],
                item["shell"])
            )
