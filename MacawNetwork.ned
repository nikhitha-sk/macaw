network MacawNetwork
{
    submodules:
        node[3]: MacawNode {
            @display("i=0,p=100,200;i=1,p=300,200;i=2,p=200,100");
        }
    connections:
        node[0].out[0] --> node[1].in[0];
        node[0].out[1] --> node[2].in[0];
        node[1].out[0] --> node[0].in[1];
        node[1].out[1] --> node[2].in[1];
        node[2].out[0] --> node[0].in[0];
        node[2].out[1] --> node[1].in[1];
}