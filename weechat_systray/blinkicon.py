#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import threading
import weechat

from signal import SIGKILL


# --
# CONSTANTS
# --
ICON_DEFAULT = '/usr/share/icons/hicolor/scalable/apps/im-irc.svg'
ICON_TRANS = '/usr/share/icons/hicolor/scalable/apps/im-icq.svg'


# --
# IMPLEMENTATION
# --
class trayer_icon(object):
    '''Implements the GTK systray icon.

    This object will run in its own process due to an issue when importing
    gtk within weechat.
    '''

    def __init__(self, pipe_read_fd):
        ''' Creates a gtk systray icon for weechat messages.

        @pipe_read_fd: read file descriptor, since it's not possible to
        import GTK within weechat we need a way to receive alerts from the
        main process to know when the icon will be changed. I'd consider other
        IPC if this script were intended to be a bit complex.
        '''
        # ugly, but...
        self._gtk = __import__('gtk')

        # this code is necessary to give gtk.main() the ability to perform in
        # a thread to avoid blocking the main process. tbh this script is not
        # interested in any GTK event.
        import gobject
        gobject.threads_init()

        # create an icon
        self._icon = self._gtk.StatusIcon()

        self._initial_img = None
        self._transient_img = None

        self._pread_fd = pipe_read_fd

    def set_icon_image(self, initial_image_path, transient_image_path):
        ''' Sets two pictures used to be displayed in systray.

        @initial_image_path: path to the main image
        @transient_image_path: path to the image shown when event happens

        @returns True
        '''
        if not os.path.exists(initial_image_path) or \
           not os.path.exists(transient_image_path) or \
           self._icon is None:
            return False

        self._initial_img = initial_image_path
        self._transient_img = transient_image_path

        self.reset()

        # run gtk main loop in a separate thread to avoid blocking. It could
        # be dangerous (as per GTK docs) if we were going to have more than
        # one thread managing the UI
        proc = threading.Thread(target=self._gtk.main)
        proc.start()
        self.main_loop()

        return True

    def main_loop(self):
        ''' Reads the pipe file descriptor written by its parent and change
        the systray icon accordingly.
        '''
        while True:
            data = os.read(self._pread_fd, 8)

            # when parent closes its pipe fd side, read() won't block but
            # send an empty data, so it need to stop this iteration
            if not data:
                break

            if data == 'SET':
                self.set()

            elif data == 'RESET':
                self.reset()

            # what to do when receiving an unexpected data? hmmm, quit...
            else:
                break

    def reset(self):
        ''' Sets the main icon in the systray.
        '''
        self._icon.set_from_file(self._initial_img)

    def set(self):
        ''' Sets the transient icon in the systray.
        '''
        self._icon.set_from_file(self._transient_img)


class weechat_script(object):
    ''' Registers the script and add the hook for any message received for the
    current user.

    When a message is arrived, the icon will be changed to alert the user
    about such message. The icon will be reset when any key is pressed in
    the weechat message bar.
    '''

    def __init__(self):
        ''' Creates the script instance and add hook, unfortunately it is
        not possible to use mathods as callbacks.
        '''
        weechat.register("SystrayIcon",
                         "Ziviani",
                         "0.1",
                         "GPLv2",
                         "Systray icon for weechat.",
                         "_shutdown_plugin",
                         "")

        weechat.hook_print("", "irc_privmsg", "", 1, "_highlight_msg_cb", "")


# --
# GLOBALS :-(  - weechat hooks doesn't allow methods.
# --
g_pipe_write_fd = None
g_child_pid = None
g_buffer_hook = None


# --
# FUNCTIONS
# --
def _shutdown_plugin():
    '''Handles plugin shutdown.
    '''
    global g_child_pid

    if g_child_pid is None:
        return weechat.WEECHAT_RC_OK

    # kill the child and collect it to avoid zombie
    os.kill(g_child_pid, SIGKILL)
    os.waitpid(g_child_pid, 0)[1]
    return weechat.WEECHAT_RC_OK


def _key_pressed_cb(data, signal, signal_data):
    '''Weechat key pressed button callback. This callback is hooked after the
    message is arrived, requiring the user attention, when the user hits any
    key in the weechat message bar the icon is reset and this event unhooked.

    https://weechat.org/files/doc/stable/weechat_plugin_api.en.html
    '''
    global g_pipe_write_fd
    global g_buffer_hook
    if g_buffer_hook is None:
        return weechat.WEECHAT_RC_OK

    try:
        os.write(g_pipe_write_fd, 'RESET')

    except OSError as e:
        weechat.unhook_all()
        weechat.prnt("", 'problem on using pipe fd, unhooking msg % s' %
                     str(e))

    weechat.unhook(g_buffer_hook)
    g_buffer_hook = None
    return weechat.WEECHAT_RC_OK


def _highlight_msg_cb(data, buffer, date, tags, displayed,
                      highlight, prefix, message):
    '''Weechat message highlight callback. This is called when a message
    arrives with user's nick in it.

    https://weechat.org/files/doc/stable/weechat_plugin_api.en.html
    '''
    if not int(highlight) and 'notify_private' not in tags:
        return weechat.WEECHAT_RC_OK

    global g_pipe_write_fd
    global g_buffer_hook
    try:
        os.write(g_pipe_write_fd, 'SET')

    except OSError as e:
        weechat.unhook_all()
        weechat.prnt('', 'problem on using pipe fd, unhooking msg' %
                     str(e))

    if g_buffer_hook is None:
        g_buffer_hook = weechat.hook_signal('key_pressed',
                                            '_key_pressed_cb',
                                            '')

    return weechat.WEECHAT_RC_OK


def main():
    '''Script entry point. It creates a child process to handle GTK code
    (because it was not possible to make it run within Weechat) and
    communicates with it by using PIPE.
    '''
    global g_child_pid
    pread, pwrite = os.pipe()

    g_child_pid = os.fork()
    if g_child_pid == 0:
        os.close(pwrite)
        ticon = trayer_icon(pread)
        ticon.set_icon_image(ICON_DEFAULT, ICON_TRANS)

    else:
        global g_pipe_write_fd
        os.close(pread)
        g_pipe_write_fd = pwrite
        weechat_script()


if __name__ == '__main__':
    main()
