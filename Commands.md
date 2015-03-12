The purpose of this page will be to document all of the commands available in naim.  Every command begins with a slash (/).  There is an help system available from within naim.  Type ```
/help``` to start.

## Connection Commands ##
### /newconn ###
  * Syntax: /newconn [label [ protocol ](.md)]
  * Examples:
  * /newconn EFnet
  * /newconn my\_conn TOC
  * Description: Creates a new connection window.  The first example creates a new connection labeled 'EFnet'.  Naim automaticly assumes that it is of type IRC.  The second example creates a new connection labeled 'my\_conn' of type TOC (which is the protocol that AIM usues)

### /delconn ###
  * Syntax: /delconn [label ](.md)
  * Examples:
  * /delconn
  * /delconn my\_conn
  * Description: Deletes/closes the specified connection.  If no connection is specified, than the currently active connection is deleted.  There must be at least one active connection.

### /connect ###
  * Syntax: /connect [name [ server [ port ](.md)]]
  * Examples:
  * /connect my\_screenname
  * /connect nick irc.servercentral.net
  * Description: Connect to a specified server.  The protocol used is dependant on the current connection

### /disconnect ###
  * Syntax: /disconnect
  * Description: Disconnect from the current server.

## Chatting Commands ##
### /say ###
  * Syntax: /say message
  * Example: /say hello there
  * Description: Sends a message to the current window.

### /msg ###
  * Syntax: /msg user message
  * Examples:
  * /msg #naim hello world!
  * /msg "naim help" help me!
  * Description: Sends a message to another user or chatroom.

### /me ###
  * Syntax: /me action
  * Example: /me is happy
  * Description: Sends an 'action' message to the current window

### /join ###
  * Syntax: /join chat [key ](.md)
  * Examples:
  * /join #myroom thepassword
  * /join 4:naim
  * Description: Joins a chatroom using the specified password, if one is needed.

### /part ###
  * Syntax: /part [window ](.md)
  * Example: /part #naim
  * Description: Closes a window, or part a chatroom.
  * Alias: /close, /part

### /invite ###
  * Syntax: /invite name [chat ](.md)
  * Description: Invites someone to a chat.

### /info ###
  * Syntax [name ](.md)
  * Description: Retrieves info about a user if specified, or the current user.  On AIM, this is the user's profile.  On IRC, this is the user's ```
/whois```
  * Aliases: /wi, /whois

## Window Commands ##
### /open ###
  * Syntax: /open name
  * Example: /open my\_buddy
  * Description: Opens a window for a buddy.  You can use this to open a window to talk to buddy not on your buddy list.
  * Alias: /window

### /close ###
  * See /part

### /closeall ###
  * Syntax: /closeall
  * Description: Closes all open windows for offline buddies.

### /jump ###
  * Syntax: /jump [windowname ](.md)
  * Example: /jump EFnet:#naim
  * Description: Jumps (switches to) the specified window.  If no window is specified, then /jump moves to the next 'active' window (represented by being colored yellow in your window list).

### /jumpback ###
  * Syntax: /jumpback
  * Description: Jumps to the previous window.

### /clear ###
  * Syntax: /clear
  * Description: Temporarily clear the scrollback for the current window.

### /clearall ###
  * Syntax: /clearall
  * Description: Performs a ```
/clear``` on all open windows.

### /resize ###
  * Syntax: /resize [height ](.md)
  * Example: /resize 200
  * Description: Resizes all windows.  Specifying a height allows you to access a large scrollback buffer, but beware that a large height will potentially take a while to re-draw.

### /winlist ###
  * Syntax: [| visible | auto](hidden.md)
  * Example: /winlist auto
  * Description: Controls the display behavoir of the window list:
  * Visible: The window list is always visible
  * Hidden: The window list is always hidden
  * Auto: The window list is only shown when switching windows, or there is a waiting active window.

## Buddylist Mangement ##
### /addbuddy ###
  * Syntax: /addbuddy buddy [group [ realname ](.md)]
  * Examples:
  * /addbuddy "naim help"
  * /addbuddy "naim help" Friends "Dan Reed"
  * Description: Adds a buddy to your permeanent buddy list.  You may specify a group for that buddy, and a "real name".  You can also use this command to change what group they're in.
  * Aliases: /add, /friend, /groupbuddy

### /delbuddy ###
  * Syntax: /delbuddy buddy
  * Description: Removes a buddy from your buddy list.

### /tagbuddy ###
  * Syntax: /tagbuddy name [note ](.md)
  * Description: Tags a buddy with a reminder message.  Use ```
/tagbuddy name``` with note to remove a note.
  * Alias: /tag

### /namebuddy ###
  * Syntax: /namebuddy name [real name ](.md)
  * Description: Changes the read name for a buddy

### /sync ###
  * Syntax: /sync
  * Descripition: Uploads your buddy list to the server for safekeeping.  N.B. this command is ''not'' guaranteed to do anything.  See ```
/save```

## IRC Specific Commands ##

### /kick ###
  * Syntax: /kick name [reason ](.md)
  * Example: /kick blahuser dont beg for ops!
  * Descriptions: removes a user from an IRC chatroom.  An optional reason maybe be given

### /op ###
  * Syntax: /op name
  * Description: Gives another user 'operator' status in the current IRC chatroom.

### /deop ###
  * Syntax: /deop name
  * Description: Removes another user 'op' status.

### /topic ###
  * Syntax: /topic [topic ](.md)
  * Description: If no topic is specified, the current room topic is displayed.  Otherwise ```
/topic``` changes the current topic.

### /ctcp ###
  * Syntax: /ctcp name [request [ message ](.md)]
  * Example:
  * /ctcp n_version
  * Description: Sends a CTCP (Client To Client Protocol) message to someone._

## Other Commands ##

### /set ###
  * Syntax: /set [varname [ value [ dummy ](.md)]]
  * Example: /set showraw 1
  * Description: View or change the value of a configureation or session variable.
  * See [[and Preferences](Settings.md)]

### /alias ###
  * Syntax: /alias commandname script
  * Examples:
  * /alias mode /msg :RAW $args1*** /alias k /msg :RAW KICK $cur $arg1 :$args2**
  * Description: Creates a new command alias
  * Accessing arguemnts:
  * $arg1 - the first argument
  * $args1**-  all arguments after the first, including the first
  * $arg2 - the second argument
  * $args2** - all arguments after the second, including the second

### /modlist ###
  * Syntax: /modlist
  * Description: Search for and list all potential and resident naim modules.

### /modload ###
  * Syntax: /modload module [options ](.md)
  * Description: Load and initialize a dynamic module.

### /modunload ###
  * Syntax: /modunload module
  * Description: Deinitialize and unload a resident module.

### /exec ###
  * Syntax: /exec [-o ](.md) command
  * Examples:
  * /exec sh /u/n/myscript.sh
  * /exec -o uname -a
  * Description: Executes a shell command.  The optional -o switch causes any output to be 'said' to the current window.

### /eval ###
  * Syntax: /eval commands
  * Example: /eval /msg #naim $lasturl
  * Description: Evaluates a command with $-variable substitution.  Can also evaluate lua commands
  * See: [[and Preferences](Settings.md)]

### /status ###
  * Syntax: /status [connection](connection.md)
  * Example: /status AIM
  * Description: Prints out various information about a specific connection.

### /offer ###
  * Syntax: /offer name filename
  * Description: [EXPERIMENTAL](EXPERIMENTAL.md) Offer a file transfer request to someone.

### /accept ###
  * Syntax: /accept filename
  * Description: [EXPERIMENTAL](EXPERIMENTAL.md) Accept a file transfer request in the current window.
