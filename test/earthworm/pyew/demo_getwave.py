#!/opt/earthworm/venv/bin/python3

"""
demo_getwave.py

Demonstrates how to use PyEW.EWModule to continuously read waveform data
from an Earthworm ring using get_wave().
"""

import time
import PyEW
import logging

logging.basicConfig(level=logging.INFO, format='%(asctime)s %(name)s %(levelname)s: %(message)s')
log = logging.getLogger(__name__)


class DemoRead:
    """Continuously reads waveform packets from an Earthworm ring."""

    def __init__(self, def_ring=1000, mod_id=150, inst_id=2, hb_time=30.0,
                 add_rings=None, debug=False):
        """
        Args:
            def_ring: Default ring ID for heartbeats and stop messages.
            mod_id:   Module ID for this instance.
            inst_id:  Installation ID.
            hb_time:  Heartbeat interval in seconds.
            add_rings: List of ring IDs to read waveforms from.
            debug:    Enable verbose debug logging in PyEW.
        """
        if add_rings is None:
            add_rings = [1000]

        self.running = False

        log.info(f"Initializing PyEW.EWModule: ring={def_ring}, mod={mod_id}, "
                 f"inst={inst_id}, hb={hb_time}, rings={add_rings}")

        self.module = PyEW.EWModule(def_ring, mod_id, inst_id, hb_time, debug)

        for ring_id in add_rings:
            try:
                self.module.add_ring(ring_id)
                log.debug(f"Ring {ring_id} added")
            except Exception as e:
                log.error(f"Failed to add ring {ring_id}: {e}")

    def process_wave(self, wave):
        """
        Process a single wave packet. Override or extend this method
        to do something useful with the data.

        Args:
            wave: dict with keys station, network, channel, location,
                  nsamp, samprate, startt, endt, datatype, data (numpy array)
        """
        log.debug(f"wave_dict: {wave}")
        wave.pop('datatype')
        wave.pop('data')
        log.info(f'wave_dict: {wave}')
        # log.info(f"{wave['network']}.{wave['station']}.{wave['channel']}.{wave['location']} "
        #      f"| samples={wave['nsamp']} rate={wave['samprate']} modid={wave['modid']} instid={wave['instid']} "
        #      f"start={wave['startt']:.3f}")
        if not any(_id in wave for _id in ['modid', 'instid']):
            log.warning('Wave dict is missing instid or modid fields')
        # log.info(f"{wave['network']}.{wave['station']}.{wave['channel']}.{wave['location']} "
        #      f"| samples={wave['nsamp']} rate={wave['samprate']} "
        #      f"start={wave['startt']:.3f}")

    def do_loop(self, buf_ring=0):
        """
        Read all available wave packets from the given ring buffer index.
        Returns the number of packets processed in this iteration.
        """
        count = 0
        while True:
            try:
                wave = self.module.get_wave(buf_ring)
            except Exception as e:
                log.warning(f"Exception while fetching wave: {e}")
                break

            if not wave:
                # No more packets available right now
                break

            self.process_wave(wave)
            count += 1

        return count

    def run(self, buf_ring=0, sleep_interval=0.05):
        """
        Main loop: continuously reads waves until the module is told to stop
        (via EW stop message or keyboard interrupt).

        Args:
            buf_ring:       Ring buffer index to read from (0-based, matches
                            order of add_ring calls).
            sleep_interval: Seconds to sleep when no data is available,
                            to avoid busy-waiting.
        """
        self.running = True
        log.info("Starting continuous wave reader loop...")

        try:
            while self.running and self.module.mod_sta():
                n = self.do_loop(buf_ring)
                if n == 0:
                    # No data this cycle, sleep briefly
                    time.sleep(sleep_interval)
        except KeyboardInterrupt:
            log.info("Keyboard interrupt received")
        finally:
            self.stop()

    def stop(self):
        """Gracefully shut down the EW module."""
        self.running = False
        log.info("Shutting down module...")
        self.module.goodbye()


if __name__ == "__main__":
    # Example usage: read from WAVE_RING (1000)
    demo = DemoRead(
        def_ring=1000,
        mod_id=150,
        inst_id=2,
        hb_time=30.0,
        add_rings=[1000],
        debug=False,
    )
    demo.run()
