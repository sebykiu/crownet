import sys, os



from flowcontrol.crownetcontrol.setup.entrypoints import get_controller_from_args
from flowcontrol.crownetcontrol.state.state_listener import VadereDefaultStateListener

sys.path.append(os.path.abspath(".."))

from flowcontrol.strategy.controller.dummy_controller import Controller
from flowcontrol.crownetcontrol.traci import constants_vadere as tc


class ChangeTarget(Controller):
    """
    Send RouteChoice only once at the start of the simulation.
    With [Config test_control001_*]:
      * message will be propagated once
      * only first wave will get the control command and change there
        target accordingly
      * second wave will use there default target
    With [Config test_control002_*]:
      * message will be propagated 30 times with 1.0s interval
      * both waves will get the message
    """

    def __init__(self):
        super().__init__()

    def initialize_connection(self, con_manager):
        self.con_manager = con_manager

    def handle_sim_step(self, sim_time, sim_state):
        sending_node = "pNode[1].densityMap.app"
        density_map = self.con_manager.domains.v_sim.get_density_map(sending_node)
        print(density_map)
        self.con_manager.next_call_at(500.0)  # do not call again (simualtion only taks ~50 seconds(

    def handle_init(self, sim_time, sim_state):
        print("DBG: ChangeTarget.handle_init()")
        self.con_manager.next_call_at(3.0)  # call first at time 1.0 s
        print("init")


if __name__ == "__main__":
    sub = VadereDefaultStateListener.with_vars(
        "persons",
        {"pos": tc.VAR_POSITION, "speed": tc.VAR_SPEED, "angle": tc.VAR_ANGLE},
        init_sub=True,
    )

    controller = ChangeTarget()

    if len(sys.argv) == 1:

        settings = [
            "--port",
            "9997",
            "--host-name",
            "0.0.0.0",
        ]

        controller = get_controller_from_args(working_dir=os.getcwd(), args=settings, controller=controller)
    else:
        controller = get_controller_from_args(
            working_dir=os.path.dirname(os.path.abspath(__file__)),
            controller=controller, )

    controller.register_state_listener("default", sub, set_default=True)
    print("start...")
    controller.start_controller()
    print("start_2...")
