class Group(object) :

    #
    # Description
    #   The class constructor
    #
    # Parameters
    #   _path - The path to "/etc/group" equivalent file
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
    #   groupname - The name of the group
    #   x         - The token indicating that the password is encrypted in /etc/shadow
    #   gid       - The group ID
    #   username  - The name of the user
    #
    # Returns
    #    A dictionary item
    #
    def __make_item(self, groupname, x, gid, username) :
        return {
            "groupname" : groupname,
            "x"         : x,
            "gid"       : gid,
            "username"  : username
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

                if len(tokens) != 4 :
                    if self.verbose :
                        print("Invalid line : {}".format(line))
                        continue

                self.add_entry(tokens[0], tokens[1], tokens[2], tokens[3].rstrip())

    #
    # Description
    #   This function adds an entry to the file
    #
    # Parameters
    #   gid       - The group ID
    #   username  - The user name
    #   groupname - The group name
    #   x         - The password encrypted token
    #
    # Returns
    #    Nothing
    #
    def add_entry(self, gid, username, groupname="evs", x="x") :

        item = self.__make_item(groupname, x, gid, username)

        if self.verbose :
            print("Adding item : {}".format(item))

        self.items.append(item)

    #
    # Description
    #   This function indicates if the group id (GID) is found in the file
    #
    # Parameters
    #   gid - The group ID
    #
    # Returns
    #    True  - The group id was found
    #    False - The group id was not found
    #
    def is_group_id_in(self, gid) :
        for item in self.items :
            if item["gid"] == gid :
                return True

        return False

    #
    # Description
    #   This function write the group content to the given path
    #
    # Parameters
    #   path - The path where to write the file
    #
    # Returns
    #    Nothing
    #
    def write_to(self, path) :
        with open(path, "w") as file :
            self.write_in(file)

    #
    # Description
    #   This function writes the group content in the given file
    #
    # Parameters
    #   file - The file object
    #
    # Returns
    #  Nothing
    #
    def write_in(self, file) :
        for item in self.items :
            file.write("{}:{}:{}:{}\n".format(
                item["groupname"],
                item["x"],
                item["gid"],
                item["username"])
            )
