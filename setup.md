Note: 4 challanges in one binary!

It is advisable to use the provided pre-compiled binary, but you can roll your own with `make`.


To deploy, use socat:
`socat tcp4-listen:2323,reuseaddr,fork exec:'./dnforth'`

Note:
- the binary expects itself to be named `dnforth` and run from their own folder (it uses this info when dumping itself)
- place the `flagz` folder in the CWD, it's used for reading the flagz from