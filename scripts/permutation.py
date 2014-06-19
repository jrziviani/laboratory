#!/usr/bin/env python
"""
This script is a laboratory for algorithms in python.
"""

def permute_iterative(str_permut):
    """
    Prints all permutations given a particular string in
    an iterative algorithm.

    @type string:  str
    @param string: string to be permuted

    @rtype:   nothing
    @returns: nothing
    """
    # string is one char long (or none), print it
    # and return
    if len(str_permut) <= 1:
        print str_permut
        return

    # insert the string in the stack
    stack = [ str_permut ]

    # store a item that will be isolated to be print
    # later
    up_header = None

    while stack:

        # when last item in the stack is a list it means we are
        # dealing with a ['a', ['bc']], where 'a' will be
        # isolated and the rest will be handle properly:
        # (1) if len == 2 print the up_header + tail ('abc') and
        #     the reverse tail ('acb')
        # (2) if len > 2 like ['a', ['bcd']], redo the process to
        #     get a ['a', ['b', ['cd']]] and, then, go to case (1)
        # (3) if len == 1 happens for len(string) <= 3, in this case
        #     the algorithm fulfill the list accordingly, no need
        #     to print the tail in reverse.
        if isinstance(stack[-1], list):
            data      = stack.pop()  # get the list from stack.
            up_header = data[0]      # extract the isolated item: 'a'.
            data      = data[1:][0]  # data is now the tail: 'bc'.

            # shortcut to keep the tail no longer than three items.
            if isinstance(data, list):
                up_header = up_header + data[0]
                data      = data[1:][0]

        # first case, simply pop data from stack.
        else:
            data = stack.pop()

        # loop the data retrieved from stack in order to go as long
        # as possible in the items to be permuted.
        for i in range(len(data)):

            head = data[i]
            tail = data[:i] + data[i + 1:]

            if up_header:

                # print a tail that has only one item.
                if len(tail) <= 1:
                    print up_header + head + ''.join(tail)
                    continue

                # print the tail both forward and reverse for
                # tail with 2 items.
                if len(tail) == 2:
                    print up_header + head + ''.join(tail)
                    print up_header + head + ''.join(tail)[::-1]
                    continue

                # expand the items if tail has more than 2 items
                stack.append( [up_header, [head, tail]] )

            # expand the list
            else:
                stack.append( [head, tail] )

        up_header = None

if __name__ == '__main__':
    STRING_TEST = '12345678'
    permute_iterative(STRING_TEST)
