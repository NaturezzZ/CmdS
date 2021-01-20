#include <core.p4>
#include <tna.p4>



/*************************************************************************
 ************* C O N S T A N T S    A N D   T Y P E S  *******************
*************************************************************************/
enum bit<16> ether_type_t {
    TPID       = 0x8100,
    IPV4       = 0x0800
}

const bit<32> k=5;

const bit<32> thresh=5;

enum bit<8>  ip_proto_t {
    ICMP  = 1,
    IGMP  = 2,
    TCP   = 6,
    UDP   = 17
}

type bit<48> mac_addr_t;

/*************************************************************************
 ***********************  H E A D E R S  *********************************
 *************************************************************************/
/*  Define all the headers the program will recognize             */
/*  The actual sets of headers processed by each gress can differ */

/* Standard ethernet header */
header ethernet_h {
    mac_addr_t    dst_addr;
    mac_addr_t    src_addr;
    ether_type_t  ether_type;
}


header ipv4_h {
    bit<4>   version;
    bit<4>   ihl;
    bit<8>   diffserv;
    bit<16>  total_len;
    bit<16>  identification;
    bit<3>   flags;
    bit<13>  frag_offset;
    bit<8>   ttl;
    bit<8>   protocol;
    bit<16>  hdr_checksum;
    bit<32>  src_addr;
    bit<32>  dst_addr;
}


header icmp_h {
    bit<16>  type_code;
    bit<16>  checksum;
}

header igmp_h {
    bit<16>  type_code;
    bit<16>  checksum;
}

header tcp_h {
    bit<16>  src_port;
    bit<16>  dst_port;
    bit<32>  seq_no;
    bit<32>  ack_no;
    bit<4>   data_offset;
    bit<4>   res;
    bit<8>   flags;
    bit<16>  window;
    bit<16>  checksum;
    bit<16>  urgent_ptr;
}

header udp_h {
    bit<16>  src_port;
    bit<16>  dst_port;
    bit<16>  len;
    bit<16>  checksum;
}

/*************************************************************************
 **************  I N G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/
 
    /***********************  H E A D E R S  ************************/

struct my_ingress_headers_t {
    ethernet_h         ethernet;

    ipv4_h             ipv4;
    icmp_h             icmp;
    igmp_h             igmp;
    tcp_h              tcp;
    udp_h              udp;
}

    /******  G L O B A L   I N G R E S S   M E T A D A T A  *********/
struct  id1_fre_t
{
    bit<32> id1;
    bit<32> fre;
}
struct  id2_ts_t
{
    bit<32> id2;
    bit<32> ts;
}
struct  id3_nexthop_t
{
    bit<32> id3;
    bit<32> nexthop;
}
struct  id4_pend_t
{
    bit<32> pad1;
    bit<32> pad2;
}
struct flow_id_t
{
bit<32>  ipsrc;
bit<32> ipdst;
bit<8> ipproto;
bit<32> lookup;
}
struct my_ingress_metadata_t {
    id1_fre_t id1_fre;
    id2_ts_t id2_ts;
    flow_id_t flow_id;
    id3_nexthop_t id3_nexthop;
    bit<32> id;
    bit<16> index;
    bit<32> ts;
    bit<4> pre;
    bit<32> nexthop;
    bit<32> ecmphop;
    bit<4> hopindex;
    bit<32> finalhop;
    bit<1> tag;
    bit<4> counterindex;
    bit<32> flag4pend;
    bit<32> flagforpend;
    bit<32> tsd;

}

    /***********************  P A R S E R  **************************/

parser IngressParser(packet_in        pkt,
    /* User */
    out my_ingress_headers_t          hdr,
    out my_ingress_metadata_t         meta,
    /* Intrinsic */
    out ingress_intrinsic_metadata_t  ig_intr_md)
{
    /* This is a mandatory state, required by Tofino Architecture */
    state start {
        pkt.extract(ig_intr_md);
        pkt.advance(PORT_METADATA_SIZE);
        meta.id=0;
        meta.index=0;
        transition parse_ethernet;
    }

    
    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        /* 
         * The explicit cast allows us to use ternary matching on
         * serializable enum
         */        
        transition select((bit<16>)hdr.ethernet.ether_type) {
            (bit<16>)ether_type_t.IPV4            :  parse_ipv4;
            default :  accept;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        meta.flow_id.ipsrc=hdr.ipv4.src_addr;
        meta.flow_id.ipdst=hdr.ipv4.dst_addr;
        meta.flow_id.ipproto=hdr.ipv4.protocol;
        meta.flow_id.lookup = pkt.lookahead<bit<32>>();
        transition select(hdr.ipv4.protocol) {
            1  : parse_icmp;
            2  : parse_igmp;
            6   : parse_tcp;
            17  : parse_udp;
            default : accept;
    }
    }
    state parse_icmp {
        pkt.extract(hdr.icmp);
        transition accept;
    }
    
    state parse_igmp {
        pkt.extract(hdr.igmp);
        transition accept;
    }
    
    state parse_tcp {
        pkt.extract(hdr.tcp);
        transition accept;
    }
    
    state parse_udp {
        pkt.extract(hdr.udp);
        transition accept;}
}

control Ingress(/* User */
    inout my_ingress_headers_t                       hdr,
    inout my_ingress_metadata_t                      meta,
    /* Intrinsic */
    in    ingress_intrinsic_metadata_t               ig_intr_md,
    in    ingress_intrinsic_metadata_from_parser_t   ig_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md,
    inout ingress_intrinsic_metadata_for_tm_t        ig_tm_md)
{
    Hash<bit<32>>(HashAlgorithm_t.CRC32) hash1;
    Hash<bit<16>>(HashAlgorithm_t.CRC16) hash2;
    Hash<bit<4>>(HashAlgorithm_t.CRC8) hash3;




 //first step for fre   
    Register<id1_fre_t, bit<16>>(0xFFFF) id1_fre_reg1;
    RegisterAction<id1_fre_t, bit<16>, bit<32>>(id1_fre_reg1) work1_fre=
    {
void apply(inout id1_fre_t register_data, out bit<32> result) {
            if (register_data.id1==meta.id)
            {
                register_data.fre=register_data.fre+k;
            }
            if (register_data.id1!=meta.id && register_data.fre!=0)
            {
                register_data.fre=register_data.fre-1;
            }
            if (register_data.id1!=meta.id && register_data.fre==0)
            {
                register_data.id1=meta.id;
               // register_data.fre=1;
            }
            result=register_data.fre;
        }
    };


//first step for id1   
    Register<id1_fre_t, bit<16>>(0xFFFF) id1_fre_reg2;
    RegisterAction<id1_fre_t, bit<16>, bit<32>>(id1_fre_reg2) work1_id1=
    {
void apply(inout id1_fre_t register_data, out bit<32> result) {
            if (register_data.id1==meta.id)
            {
                register_data.fre=register_data.fre+k;
            }
            if (register_data.id1!=meta.id && register_data.fre!=0)
            {
                register_data.fre=register_data.fre-1;
            }
            if (register_data.id1!=meta.id && register_data.fre==0)
            {
                register_data.id1=meta.id;
               // register_data.fre=1;
            }
            result=register_data.id1;
        }
    };
//second step for predicate
Register<id2_ts_t, bit<16>>(0xFFFF) id2_ts_reg1;
RegisterAction<id2_ts_t, bit<16>, bit<32>>(id2_ts_reg1) work2_pre1=
    {
void apply(inout id2_ts_t register_data, out bit<32> result) {
           
           result=register_data.ts;
           if (meta.ts>register_data.ts+thresh)
            {
                register_data.ts=meta.ts;
            }
            
            if (meta.ts>register_data.ts+thresh &&meta.id!=register_data.id2)
            {
                register_data.id2=meta.id;
            }
            if (meta.ts<=register_data.ts+thresh &&meta.id==register_data.id2)
            {
                register_data.ts=meta.ts|0x80000000;

            }
            
        }
    };

Register<id2_ts_t, bit<16>>(0xFFFF) id2_ts_reg2;
RegisterAction<id2_ts_t, bit<16>, bit<32>>(id2_ts_reg2) work2_pre2=
    {
void apply(inout id2_ts_t register_data, out bit<32> result) {
           
           result=register_data.id2;
           if (meta.ts>register_data.ts+thresh)
            {
                register_data.ts=meta.ts;
            }
            
            if (meta.ts>register_data.ts+thresh &&meta.id!=register_data.id2)
            {
                register_data.id2=meta.id;
            }
            if (meta.ts<=register_data.ts+thresh &&meta.id==register_data.id2)
            {
                register_data.ts=meta.ts|0x80000000;

            }
            
        }
    };
action fl(bit<32> hop)
{
    meta.nexthop=hop;
}
//counter for least Counter<>
table findleast//找到最小的counter
{   key={meta.tag:exact;}
    actions={NoAction;fl;}
    default_action=NoAction;
}

action fi(bit<4> index)
{
    meta.counterindex=index;
}
//counter for least Counter<>
table findindex//用端口号找到对应的counter
{   key={meta.finalhop:exact;}
    actions={NoAction;fi;}
    default_action=NoAction;
}


Counter<bit<64>,bit<4>>(16,CounterType_t.BYTES) c;

action calid()//计算32位指纹id
{
meta.id=hash1.get({meta.flow_id.ipsrc,meta.flow_id.ipdst,meta.flow_id.ipproto,meta.flow_id.lookup});
}
table calid_t
{
    actions={calid;}
    default_action=calid;
}

action calindex()//计算index
{
meta.index=hash2.get({meta.flow_id.ipsrc,meta.flow_id.ipdst,meta.flow_id.ipproto,meta.flow_id.lookup});
}
table calindex_t
{
    actions={calindex;}
    default_action=calindex;
}
action calnexthop()//计算ecmp下的nexthop的index
{
meta.hopindex=hash3.get({meta.flow_id.ipsrc,meta.flow_id.ipdst,meta.flow_id.ipproto,meta.flow_id.lookup});
}
table calnexthop_t
{
    actions={calnexthop;}
    default_action=calnexthop;
}

action findecmphop(bit<32> nh)
{
    meta.ecmphop=nh;
}
table findecmphop_t//用ecmphop index找到对应的端口号
{
    key={meta.hopindex:exact;}
    actions={NoAction;findecmphop;}
    default_action=NoAction;
}

action setegressport()//设置egress port
{
ig_tm_md.ucast_egress_port=(bit<9>)meta.finalhop;
}
table setegressport_t
{
    actions={setegressport;}
    default_action=setegressport;
}



//third step for scheduling
Register<id3_nexthop_t, bit<16>>(0xFFFF) id3_nexthop_reg;
RegisterAction<id3_nexthop_t, bit<16>, bit<32>>(id3_nexthop_reg) work3_sch=
 {
void apply(inout id3_nexthop_t register_data, out bit<32> result) {

           if (meta.nexthop!=0)
           {
               register_data.nexthop=meta.nexthop;
           }
            
            result=register_data.nexthop;
        }
    };

/*Register<id4_pend_t, bit<16>>(0xFFFF) pend_reg;
RegisterAction<id4_pend_t, bit<16>, bit<32>>(pend_reg) work4_pend=
 {
void apply(inout id4_pend_t register_data, out bit<32> result) {

           if (meta.id2_ts.ts>0)
           {
               register_data.pad1=1;
           }
           
            
            
        }
    };*/

action getflag()
{
//meta.flagforpend=
meta.flag4pend=max(meta.tsd,meta.ts);
}
table getflag_t
{
    actions={getflag;}
    default_action=getflag;
}
    apply
    {
        bit<1> sch=0;
        bit<1> flagforpend2=0;
        bit<8> flagforpend=0;
        //bit<16> index=hash2.get({meta.flow_id.ipsrc,meta.flow_id.ipdst,meta.flow_id.ipproto,meta.flow_id.lookup});
        calid_t.apply();
        calindex_t.apply();
        calnexthop_t.apply();
        findecmphop_t.apply();
        meta.tag=1;
        //meta.index=index;
        meta.finalhop=0;
        meta.ts=(bit<32>)ig_intr_md.ingress_mac_tstamp;
        
        meta.id1_fre.fre=work1_fre.execute(meta.index);
        meta.id1_fre.id1=work1_id1.execute(meta.index);
        if (meta.id1_fre.id1!=meta.id)
        {
            meta.ts=meta.ts&0x7fffffff;
        }
        else
        {
            meta.ts=meta.ts|0x80000000;
        }
       meta.id2_ts.ts=work2_pre1.execute(meta.index);
       meta.id2_ts.id2=work2_pre2.execute(meta.index);

       if (meta.id==meta.id2_ts.id2)
       {
           flagforpend2=1;
       }
       meta.tsd=meta.id2_ts.ts+thresh;
       getflag_t.apply();
       if (meta.flag4pend==meta.ts)
       {
           if (flagforpend2==1)
           {
               findleast.apply();
               sch=1;
           }
           else
           {
               meta.nexthop=meta.ecmphop;
               sch=1;
           }
       }
       else
       {
           if (flagforpend2==1)
           {
               sch=1;
               meta.nexthop=0;
           }
           else
           {
               meta.nexthop=meta.ecmphop;
           }
       }
       if (sch==1)
       {
        meta.finalhop=work3_sch.execute(meta.index);
       }
       else
       {
           meta.finalhop=meta.ecmphop;
       }

setegressport_t.apply();

findindex.apply();
c.count(meta.counterindex);
    }

}

control IngressDeparser(packet_out pkt,
    /* User */
    inout my_ingress_headers_t                       hdr,
    in    my_ingress_metadata_t                      meta,
    /* Intrinsic */
    in    ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md)
{
    apply {
        pkt.emit(hdr);
    }
}
/*************************************************************************
 ****************  E G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/

    /***********************  H E A D E R S  ************************/

struct my_egress_headers_t {
}

    /********  G L O B A L   E G R E S S   M E T A D A T A  *********/

struct my_egress_metadata_t {
}

    /***********************  P A R S E R  **************************/

parser EgressParser(packet_in        pkt,
    /* User */
    out my_egress_headers_t          hdr,
    out my_egress_metadata_t         meta,
    /* Intrinsic */
    out egress_intrinsic_metadata_t  eg_intr_md)
{
    /* This is a mandatory state, required by Tofino Architecture */
    state start {
        pkt.extract(eg_intr_md);
        transition accept;
    }
}

    /***************** M A T C H - A C T I O N  *********************/

control Egress(
    /* User */
    inout my_egress_headers_t                          hdr,
    inout my_egress_metadata_t                         meta,
    /* Intrinsic */    
    in    egress_intrinsic_metadata_t                  eg_intr_md,
    in    egress_intrinsic_metadata_from_parser_t      eg_prsr_md,
    inout egress_intrinsic_metadata_for_deparser_t     eg_dprsr_md,
    inout egress_intrinsic_metadata_for_output_port_t  eg_oport_md)
{
    apply {
    }
}

    /*********************  D E P A R S E R  ************************/

control EgressDeparser(packet_out pkt,
    /* User */
    inout my_egress_headers_t                       hdr,
    in    my_egress_metadata_t                      meta,
    /* Intrinsic */
    in    egress_intrinsic_metadata_for_deparser_t  eg_dprsr_md)
{
    apply {
        pkt.emit(hdr);
    }
}


/************ F I N A L   P A C K A G E ******************************/
Pipeline(
    IngressParser(),
    Ingress(),
    IngressDeparser(),
    EgressParser(),
    Egress(),
    EgressDeparser()
) pipe;

Switch(pipe) main;
