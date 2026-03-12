network MacawNetwork
{
    submodules:
        node[3]: MacawNode {
            @display("i=0,p=100,200;i=1,p=300,200;i=2,p=200,100");
        }
    connections:
        node[0].out --> node[1].in;
        node[1].out --> node[2].in;
        node[2].out --> node[0].in;
}