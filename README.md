# Inter Process Communication Library for AWS and Unreal Engine processes
This is still a heavy work in progress. A proper README will be written soon.

This library will allow you to commuicate player data between different processes
on the same system. You will be able to GET/SET things in AWS DynamoDB, and also
authenticate players using AWS Cognito.

Presently the different attributes that you would either GET/SET in DynabmoDB
are hardcoded to make it easier to develope the IPC portion of the library.
You will be able to define your own attributes in the future.

The entire library is multithread, and thus the types are written to be thread-safe.

The library works by accumulating GET/SET requests in different buffers, then threads
that manage the buffers will write them to file at a given tick-rate. Then other threads
read the requests into memory, where you can then manage the requests.
