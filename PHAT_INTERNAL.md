## PhatFS

In Phat/Joan, everything is a JSON object: files, handles, locks, metadata, ... even the internal filesystem data structure.
Since JSON is a meta-format, we need to define some internal structure to interact with these objects.
Note: this is for internal use only. Users should just treat all of these as opaque blobs.

#### Handle:
`array( string path )`

Path strings are UNIX-style, `/`-separated directories.

#### Metadata:
`array( string name, int filetype )`

Filetype is either `file` or `dir`.

*Will likely need to edit this to deal with locks and whether a file is open or not...*

#### Sequencer:
**NYI**

#### Locktype:
**NYI**

#### Filesystem:
`{
    String name: 
        array(
            Metadata,
            Contents)
}`

<pre>Contents := 
    | file is `dir`: filesystem 
    | file is `file`: byte stream
</pre>

**Examples:**

Empty root: <pre>
Filesystem root = {
    "/": array(
        array("/",DIR),
        {}
    )
}</pre>

Root with a file in it:
<pre>
Filesystem files = {
    "/": array(
        array("/",DIR),
        {
            "hw.txt": array(
                array("hw.txt",FILE),
                "Hello, World!"
            )
        }
    )
}
</pre>

## PhatRPC

Phat RPC messages are also Json objects. Thus, we need structure for them.

General note: Phat/Joan tries to report errors back through the RPC reply channel.
Error reply: `array( string tag:"NACK", string reason )`

The reason field gives us an idea of why we're getting a `NACK` and what to do about it.
For instance, the `NYI` reason tells us that we're lazy programmers who haven't written code to do the thing that was asked.

#### (internal) get_master

Request: `array( string tag:"get_master" )`
Reply: `array( string tag:"ACK" )`
Reply: `array( string tag:"NACK", string reason:"NOT_MASTER", int ip | string hostname, int port )`
Reply: `array( string tag:"NACK", string reason:"OLD_EPOCH", int epoch )`

Could be one of two replies. The `"NACK"` form is a general type of message that could be received from a lot of different places, which is why we want the `reason` field.

#### (internal) get_replica_list
**NYI**

#### getroot

Request: `array( string tag:"getroot" )`
Reply: `array( string root:"/" )`

This should always return the same thing, unless something has gone wrong, in which case it might be a `"NACK"`.

## Epoch Numbers

Inspired by [Paxos Made Live](http://www.cs.utexas.edu/users/lorenzo/corsi/cs380d/papers/paper2-1.pdf).

The need for epoch numbers arises from the fact that a master may think it's the master even when it is not (in the event of a master going down and coming back up, or just that someone else is elected while it is serving a client's request).

Two requests for the same epoch number will return the same value if and only if the same replica has been master then entire time between the two requests.  

The epoch number is stored within a phat_server, and is persisted to disk in the snapshot (**NYI**).

We increase the epoch number every time a new master has been elected.  In order to ensure consistency across all replicas, we include the epoch number in all paxos instances.