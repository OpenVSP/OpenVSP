
class BaseUI:
    """
    Base class to be inherited by all pyVSPAero classes.
    Contains common methods useful for many classes.
    """
    # All classes default to an empty options, unless otherwise overridden
    default_options = {}

    # pyVSPAero classes will always write/read from the VSP component set
    VSPAERO_VSP_SET = 3

    def set_option(self, name, value):
        """
        Set a solver option value. The name is not case sensitive.

        Parameters
        ----------
        name : str
            Name of option to modify

        value : depends on option
            New option value to set
        """
        name = name.lower()

        # Try to the option in the option dictionary
        try:
            self.default_options[name]
        except KeyError:
            self._vspaero_warning(f'\'{name}\' is not a valid option')
            return

        # Now we know the option exists, lets check if the type is ok:
        #        if type(value) == self.options[name][0]:
        if isinstance(value, self.options[name][0]):
            # Just set:
            description = self.default_options[name][2]
            self.options[name] = [type(value), value, description]
        else:
            raise self._vspaero_error("Datatype for Option %s was not valid. "
                        "Expected data type is %s. Received data type "
                        " is %s." % (name, self.options[name][0], type(value)))

    def set_options_to_default(self):
        # Options dictionary already exists
        if hasattr(self, "options"):
            for name in self.default_options:
                self.set_option(name, self.default_options[name][1])
        # Options dictionary hasn't been created yet
        else:
            self.options = self.default_options.copy()

    def get_option(self, name):
        """
        Get a solver option value. The name is not case sensitive.

        Parameters
        ----------
        name : str
            Name of option to get
        """

        # Redefine the get_option def from the base class so we can
        # make sure the name is lowercase

        if name.lower() in self.default_options:
            return self.options[name.lower()][1]
        else:
            raise AttributeError(repr(name) + ' is not a valid option name')

    def print_options(self):
        """
        Prints a nicely formatted dictionary of all the current solver
        options to the stdout on the root processor
        """
        # Class name
        header = type(self).__name__
        if hasattr(self, 'name'):
            # Append problem name, if BaseProblem
            header += f" ('{self.name}')"
        self._pp("+----------------------------------------+")
        self._pp("|" + f"{header} options:".center(40) + "|")
        self._pp("+----------------------------------------+")
        for name in self.options:
            if name != 'defaults':
                if self.options[name][0] == str:
                    self._pp(f"'{name}': '{self.options[name][1]}'")
                else:
                    self._pp(f"'{name}': {self.options[name][1]}")
                # print description
                self._pp(f"\t {self.options[name][2]}")

    @classmethod
    def print_default_options(cls):
        """
        Prints a nicely formatted dictionary of all the default solver
        options to the stdout
        """
        # Class name
        header = cls.__name__
        print("+----------------------------------------+")
        print("|" + f"{header} default options:".center(40) + "|")
        print("+----------------------------------------+")
        if hasattr(cls, 'default_options'):
            for name in cls.default_options:
                if cls.default_options[name][0] == str:
                    print(f"'{name}': '{cls.default_options[name][1]}'")
                else:
                    print(f"'{name}': {cls.default_options[name][1]}")
                # print description
                print(f"\t {cls.default_options[name][2]}")

    def _vspaero_warning(self, message):
        """
        Format a class-specific warning for message
        """
        # Class name
        header = type(self).__name__
        if hasattr(self, 'name'):
            # Append problem name, if BaseProblem
            header += f" ('{self.name}')"
        msg = '\n+' + '-' * 78 + '+' + '\n'
        object_warning = f'| {header} Warning: '
        msg += object_warning
        i = len(object_warning) - 1
        for word in message.split():
            if len(word) + i + 1 > 78:  # Finish line and start new one
                msg += ' ' * (78 - i) + '|\n| ' + word + ' '
                i = 1 + len(word) + 1
            else:
                msg += word + ' '
                i += len(word) + 1
        msg += ' ' * (78 - i) + '|\n' + '+' + '-' * 78 + '+' + '\n'
        print(msg)

    def _vspaero_error(self, message):
        """
        Format a class-specific error for message
        """
        # Class name
        header = type(self).__name__
        if hasattr(self, 'name'):
            # Append problem name, if BaseProblem
            header += f" ('{self.name}')"
        return Error(header, message)

class Error(Exception):
    """
    Format the error message in a box to make it clear this
    was a explicitly raised exception.
    """

    def __init__(self, obj_name, message):
        msg = '\n+' + '-' * 78 + '+' + '\n'
        object_error = '| %s Error: '%(obj_name)
        msg += object_error
        i = len(object_error) - 1
        for word in message.split():
            if len(word) + i + 1 > 78:  # Finish line and start new one
                msg += ' ' * (78 - i) + '|\n| ' + word + ' '
                i = 1 + len(word) + 1
            else:
                msg += word + ' '
                i += len(word) + 1
        msg += ' ' * (78 - i) + '|\n' + '+' + '-' * 78 + '+' + '\n'
        print(msg)
        Exception.__init__(self)
