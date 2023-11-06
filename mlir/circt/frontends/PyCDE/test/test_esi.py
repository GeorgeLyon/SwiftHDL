# RUN: rm -rf %t
# RUN: %PYTHON% %s %t 2>&1 | FileCheck %s

from pycde import (Clock, Input, InputChannel, OutputChannel, Module, generator,
                   types)
from pycde import esi
from pycde.common import Output, RecvBundle, SendBundle
from pycde.constructs import Wire
from pycde.types import (Bits, Bundle, BundledChannel, Channel,
                         ChannelDirection, ChannelSignaling, UInt, ClockType)
from pycde.testing import unittestmodule
from pycde.signals import BitVectorSignal, ChannelSignal


@esi.ServiceDecl
class HostComms:
  to_host = esi.ToServer(types.any)
  from_host = esi.FromServer(types.any)
  req_resp = esi.ToFromServer(to_server_type=types.i16,
                              to_client_type=types.i32)


class Producer(Module):
  clk = Clock()
  int_out = OutputChannel(types.i32)

  @generator
  def construct(ports):
    chan = HostComms.from_host("loopback_in", types.i32)
    ports.int_out = chan


class Consumer(Module):
  clk = Clock()
  int_in = InputChannel(types.i32)

  @generator
  def construct(ports):
    HostComms.to_host(ports.int_in, "loopback_out")


# CHECK-LABEL: hw.module @LoopbackTop(in %clk : !seq.clock, in %rst : i1)
# CHECK:         %Producer.int_out = hw.instance "Producer" sym @Producer @Producer(clk: %clk: !seq.clock) -> (int_out: !esi.channel<i32>)
# CHECK:         hw.instance "Consumer" sym @Consumer @Consumer(clk: %clk: !seq.clock, int_in: %Producer.int_out: !esi.channel<i32>) -> (
# CHECK:         esi.service.instance svc @HostComms impl as "cosim"(%clk, %rst) : (!seq.clock, i1) -> ()
# CHECK:         hw.output
# CHECK-LABEL: hw.module @Producer(in %clk : !seq.clock, out int_out : !esi.channel<i32>)
# CHECK:         [[R0:%.+]] = esi.service.req.to_client <@HostComms::@from_host>(["loopback_in"]) : !esi.channel<i32>
# CHECK:         hw.output [[R0]] : !esi.channel<i32>
# CHECK-LABEL: hw.module @Consumer(in %clk : !seq.clock, in %int_in : !esi.channel<i32>)
# CHECK:         esi.service.req.to_server %int_in -> <@HostComms::@to_host>(["loopback_out"]) : !esi.channel<i32>
# CHECK:         hw.output
# CHECK-LABEL: esi.service.decl @HostComms {
# CHECK:         esi.service.to_server @to_host : !esi.channel<!esi.any>
# CHECK:         esi.service.to_client @from_host : !esi.channel<!esi.any>


@unittestmodule(print=True)
class LoopbackTop(Module):
  clk = Clock()
  rst = Input(types.i1)

  @generator
  def construct(ports):
    p = Producer(clk=ports.clk)
    Consumer(clk=ports.clk, int_in=p.int_out)
    # Use Cosim to implement the standard 'HostComms' service.
    esi.Cosim(HostComms, ports.clk, ports.rst)


# CHECK-LABEL: hw.module @LoopbackInOutTop(in %clk : !seq.clock, in %rst : i1)
# CHECK:         esi.service.instance svc @HostComms impl as "cosim"(%clk, %rst) : (!seq.clock, i1) -> ()
# CHECK:         %0 = esi.service.req.inout %chanOutput -> <@HostComms::@req_resp>(["loopback_inout"]) : !esi.channel<i16> -> !esi.channel<i32>
# CHECK:         %rawOutput, %valid = esi.unwrap.vr %0, %ready : i32
# CHECK:         %1 = comb.extract %rawOutput from 0 : (i32) -> i16
# CHECK:         %chanOutput, %ready = esi.wrap.vr %1, %valid : i16
# CHECK:         hw.output
@unittestmodule(print=True)
class LoopbackInOutTop(Module):
  clk = Clock()
  rst = Input(types.i1)

  @generator
  def construct(self):
    # Use Cosim to implement the standard 'HostComms' service.
    esi.Cosim(HostComms, self.clk, self.rst)

    loopback = Wire(types.channel(types.i16))
    from_host = HostComms.req_resp(loopback, "loopback_inout")
    ready = Wire(types.i1)
    wide_data, valid = from_host.unwrap(ready)
    data = wide_data[0:16]
    data_chan, data_ready = loopback.type.wrap(data, valid)
    ready.assign(data_ready)
    loopback.assign(data_chan)


# CHECK-LABEL:  esi.pure_module @LoopbackInOutPure
# CHECK:          [[r0:%.+]] = esi.service.req.to_client <@HostComms::@from_host>(["loopback_in"]) : !esi.channel<i16>
# CHECK:          esi.service.req.to_server [[r0]] -> <@HostComms::@to_host>(["loopback"]) : !esi.channel<i16>
@unittestmodule(print=True)
class LoopbackInOutPure(esi.PureModule):

  @generator
  def construct(self):
    HostComms.to_host(HostComms.from_host("loopback_in", types.i16), "loopback")


class MultiplexerService(esi.ServiceImplementation):
  clk = Clock()
  rst = Input(types.i1)

  # Underlying channel is an untyped, 256-bit LI channel.
  trunk_in = Input(types.i256)
  trunk_in_valid = Input(types.i1)
  trunk_in_ready = Output(types.i1)
  trunk_out = Output(types.i256)
  trunk_out_valid = Output(types.i1)
  trunk_out_ready = Input(types.i1)

  @generator
  def generate(self, channels):

    input_reqs = channels.to_server_reqs
    if len(input_reqs) > 1:
      raise Exception("Multiple to_server requests not supported")
    MultiplexerService.unwrap_and_pad(self, input_reqs[0])

    output_reqs = channels.to_client_reqs
    if len(output_reqs) > 1:
      raise Exception("Multiple to_client requests not supported")
    output_req = output_reqs[0]
    output_chan, ready = MultiplexerService.slice_and_wrap(
        self, output_req.type)
    output_req.assign(output_chan)
    self.trunk_in_ready = ready

  @staticmethod
  def slice_and_wrap(ports, channel_type: Channel):
    assert (channel_type.inner_type.width <= 256)
    sliced = ports.trunk_in[:channel_type.inner_type.width]
    return channel_type.wrap(sliced, ports.trunk_in_valid)

  @staticmethod
  def unwrap_and_pad(ports, input_channel: ChannelSignal):
    """
    Unwrap the input channel and pad it to 256 bits.
    """
    (data, valid) = input_channel.unwrap(ports.trunk_out_ready)
    assert isinstance(data, BitVectorSignal)
    assert len(data) <= 256
    ports.trunk_out = data.pad_or_truncate(256)
    ports.trunk_out_valid = valid


# CHECK-LABEL: hw.module @MultiplexerTop{{.*}}(in %clk : !seq.clock, in %rst : i1, in %trunk_in : i256, in %trunk_in_valid : i1, in %trunk_out_ready : i1, out trunk_in_ready : i1, out trunk_out : i256, out trunk_out_valid : i1)
# CHECK:         %c0_i224 = hw.constant 0 : i224
# CHECK:         [[r0:%.+]] = comb.concat %c0_i224, %Consumer.loopback_out : i224, i32
# CHECK:         [[r1:%.+]] = comb.extract %trunk_in from 0 {sv.namehint = "trunk_in_0upto32"} : (i256) -> i32
# CHECK:         %Producer.loopback_in_ready, %Producer.int_out, %Producer.int_out_valid = hw.instance "Producer" sym @Producer @Producer{{.*}}(clk: %clk: i1, loopback_in: [[r1]]: i32, loopback_in_valid: %trunk_in_valid: i1, int_out_ready: %Consumer.int_in_ready: i1) -> (loopback_in_ready: i1, int_out: i32, int_out_valid: i1)
# CHECK:         %Consumer.int_in_ready, %Consumer.loopback_out, %Consumer.loopback_out_valid = hw.instance "Consumer" sym @Consumer @Consumer{{.*}}(clk: %clk: i1, int_in: %Producer.int_out: i32, int_in_valid: %Producer.int_out_valid: i1, loopback_out_ready: %trunk_out_ready: i1) -> (int_in_ready: i1, loopback_out: i32, loopback_out_valid: i1)
# CHECK:         hw.output %Producer.loopback_in_ready, [[r0]], %Consumer.loopback_out_valid : i1, i256, i1
# CHECK-LABEL: hw.module @Producer{{.*}}(in %clk : i1, in %loopback_in : i32, in %loopback_in_valid : i1, in %int_out_ready : i1, out loopback_in_ready : i1, out int_out : i32, out int_out_valid : i1)
# CHECK:         hw.output %int_out_ready, %loopback_in, %loopback_in_valid : i1, i32, i1
# CHECK-LABEL: hw.module @Consumer{{.*}}(in %clk : i1, in %int_in : i32, in %int_in_valid : i1, in %loopback_out_ready : i1, out int_in_ready : i1, out loopback_out : i32, out loopback_out_valid : i1)
# CHECK:         hw.output %loopback_out_ready, %int_in, %int_in_valid : i1, i32, i1


@unittestmodule(run_passes=True, print_after_passes=True, emit_outputs=True)
class MultiplexerTop(Module):
  clk = Clock()
  rst = Input(types.i1)

  trunk_in = Input(types.i256)
  trunk_in_valid = Input(types.i1)
  trunk_in_ready = Output(types.i1)
  trunk_out = Output(types.i256)
  trunk_out_valid = Output(types.i1)
  trunk_out_ready = Input(types.i1)

  @generator
  def construct(ports):
    m = MultiplexerService(HostComms,
                           clk=ports.clk,
                           rst=ports.rst,
                           trunk_in=ports.trunk_in,
                           trunk_in_valid=ports.trunk_in_valid,
                           trunk_out_ready=ports.trunk_out_ready)

    ports.trunk_in_ready = m.trunk_in_ready
    ports.trunk_out = m.trunk_out
    ports.trunk_out_valid = m.trunk_out_valid

    p = Producer(clk=ports.clk)
    Consumer(clk=ports.clk, int_in=p.int_out)


class PassUpService(esi.ServiceImplementation):

  @generator
  def generate(self, channels):
    for req in channels.to_server_reqs:
      name = "out_" + "_".join(req.client_name)
      esi.PureModule.output_port(name, req)
    for req in channels.to_client_reqs:
      name = "in_" + "_".join(req.client_name)
      req.assign(esi.PureModule.input_port(name, req.type))


# CHECK-LABEL:  hw.module @PureTest<FOO: i5, STR: none>(in %in_Producer_loopback_in : i32, in %in_Producer_loopback_in_valid : i1, in %in_prod2_loopback_in : i32, in %in_prod2_loopback_in_valid : i1, in %clk : i1, in %out_Consumer_loopback_out_ready : i1, in %p2_int_ready : i1, out in_Producer_loopback_in_ready : i1, out in_prod2_loopback_in_ready : i1, out out_Consumer_loopback_out : i32, out out_Consumer_loopback_out_valid : i1, out p2_int : i32, out p2_int_valid : i1)
# CHECK-NEXT:     %Producer.loopback_in_ready, %Producer.int_out, %Producer.int_out_valid = hw.instance "Producer" sym @Producer @Producer{{.*}}(clk: %clk: i1, loopback_in: %in_Producer_loopback_in: i32, loopback_in_valid: %in_Producer_loopback_in_valid: i1, int_out_ready: %Consumer.int_in_ready: i1) -> (loopback_in_ready: i1, int_out: i32, int_out_valid: i1)
# CHECK-NEXT:     %Consumer.int_in_ready, %Consumer.loopback_out, %Consumer.loopback_out_valid = hw.instance "Consumer" sym @Consumer @Consumer{{.*}}(clk: %clk: i1, int_in: %Producer.int_out: i32, int_in_valid: %Producer.int_out_valid: i1, loopback_out_ready: %out_Consumer_loopback_out_ready: i1) -> (int_in_ready: i1, loopback_out: i32, loopback_out_valid: i1)
# CHECK-NEXT:     %prod2.loopback_in_ready, %prod2.int_out, %prod2.int_out_valid = hw.instance "prod2" sym @prod2 @Producer{{.*}}(clk: %clk: i1, loopback_in: %in_prod2_loopback_in: i32, loopback_in_valid: %in_prod2_loopback_in_valid: i1, int_out_ready: %p2_int_ready: i1) -> (loopback_in_ready: i1, int_out: i32, int_out_valid: i1)
# CHECK-NEXT:     hw.output %Producer.loopback_in_ready, %prod2.loopback_in_ready, %Consumer.loopback_out, %Consumer.loopback_out_valid, %prod2.int_out, %prod2.int_out_valid : i1, i1, i32, i1, i32, i1
@unittestmodule(run_passes=True, print_after_passes=True, emit_outputs=True)
class PureTest(esi.PureModule):

  @generator
  def construct(ports):
    PassUpService(None)

    clk = esi.PureModule.input_port("clk", ClockType())
    p = Producer(clk=clk)
    Consumer(clk=clk, int_in=p.int_out)
    p2 = Producer(clk=clk, instance_name="prod2")
    esi.PureModule.output_port("p2_int", p2.int_out)
    esi.PureModule.param("FOO", Bits(5))
    esi.PureModule.param("STR")


# CHECK-LABEL:  hw.module @FIFOSignalingMod(in %a : !esi.channel<i32, FIFO0>, out x : !esi.channel<i32, FIFO0>)
# CHECK-NEXT:     %data, %empty = esi.unwrap.fifo %a, %rden : !esi.channel<i32, FIFO0>
# CHECK-NEXT:     %chanOutput, %rden = esi.wrap.fifo %data, %empty : !esi.channel<i32, FIFO0>
# CHECK-NEXT:     hw.output %chanOutput : !esi.channel<i32, FIFO0>
@unittestmodule(print=True)
class FIFOSignalingMod(Module):
  a = InputChannel(Bits(32), ChannelSignaling.FIFO0)
  x = OutputChannel(Bits(32), ChannelSignaling.FIFO0)

  @generator
  def build(self):
    rden_wire = Wire(types.i1)
    data, empty = self.a.unwrap(rden_wire)
    chan, rden = self.a.type.wrap(data, empty)
    rden_wire.assign(rden)
    self.x = chan


ExStruct = types.struct({
    'a': Bits(4),
    'b': UInt(32),
})


# TODO: figure out a replacement for `esi.FlattenStructPorts`.
# XFAIL-LABEL:  hw.module @FlattenTest{{.*}}(in %a_a : i4, in %a_b : ui32, in %a_valid : i1, out a_ready : i1)
@unittestmodule(print=False, run_passes=True, print_after_passes=True)
class FlattenTest(Module):
  a = InputChannel(ExStruct)

  Attributes = {esi.FlattenStructPorts}

  @generator
  def build(self):
    pass


# XFAIL-LABEL:  hw.module.extern @FlattenExternTest{{.*}}(%a_a: i4, %a_b: ui32, %a_valid: i1) -> (a_ready: i1)
@unittestmodule(print=False, run_passes=True, print_after_passes=True)
class FlattenExternTest(Module):
  a = InputChannel(ExStruct)

  Attributes = {esi.FlattenStructPorts}


# XFAIL-LABEL:   hw.module @FlattenPureTest(%a_a: i4, %a_b: ui32, %a_valid: i1) -> (a_ready: i1) attributes {esi.portFlattenStructs}
@unittestmodule(print=False, run_passes=True, print_after_passes=True)
class FlattenPureTest(esi.PureModule):

  Attributes = {esi.FlattenStructPorts}

  @generator
  def build(self):
    esi.PureModule.input_port("a", types.channel(ExStruct))


Bundle1 = Bundle([
    BundledChannel("req", ChannelDirection.TO, types.channel(types.i32)),
    BundledChannel("resp", ChannelDirection.FROM, types.channel(types.i1)),
])
# CHECK: Bundle<[('req', ChannelDirection.TO, Channel<Bits<32>, ValidReady>), ('resp', ChannelDirection.FROM, Channel<Bits<1>, ValidReady>)]>
print(Bundle1)


# CHECK-LABEL:  hw.module @SendBundleTest(in %s1_in : !esi.channel<i32>, out b_send : !esi.bundle<[!esi.channel<i32> to "req", !esi.channel<i1> from "resp"]>, out i1_out : !esi.channel<i1>) attributes {output_file = #hw.output_file<"SendBundleTest.sv", includeReplicatedOps>} {
# CHECK-NEXT:     %bundle, %resp = esi.bundle.pack %s1_in : !esi.bundle<[!esi.channel<i32> to "req", !esi.channel<i1> from "resp"]>
# CHECK-NEXT:     hw.output %bundle, %resp : !esi.bundle<[!esi.channel<i32> to "req", !esi.channel<i1> from "resp"]>, !esi.channel<i1>
@unittestmodule()
class SendBundleTest(Module):
  b_send = SendBundle(Bundle1)
  s1_in = InputChannel(types.i32)
  i1_out = OutputChannel(types.i1)

  @generator
  def build(self):
    (self.b_send, from_chans) = Bundle1.pack(req=self.s1_in)
    self.i1_out = from_chans['resp']


# CHECK-LABEL:  hw.module @RecvBundleTest(in %b_recv : !esi.bundle<[!esi.channel<i32> to "req", !esi.channel<i1> from "resp"]>, in %i1_in : !esi.channel<i1>, out s1_out : !esi.channel<i32>) attributes {output_file = #hw.output_file<"RecvBundleTest.sv", includeReplicatedOps>} {
# CHECK-NEXT:     %req = esi.bundle.unpack %i1_in from %b_recv : !esi.bundle<[!esi.channel<i32> to "req", !esi.channel<i1> from "resp"]>
# CHECK-NEXT:     hw.output %req : !esi.channel<i32>
@unittestmodule()
class RecvBundleTest(Module):
  b_recv = RecvBundle(Bundle1)
  s1_out = OutputChannel(types.i32)
  i1_in = InputChannel(types.i1)

  @generator
  def build(self):
    to_channels = self.b_recv.unpack(resp=self.i1_in)
    self.s1_out = to_channels['req']
