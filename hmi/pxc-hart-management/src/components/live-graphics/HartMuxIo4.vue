<template>
<!-- 4-channel block -->
<div class="io4-grid">
    <div class="hart-facade">
        <img src="../assets/img/hart_mux_io4_facade.png" />
    </div>

    <hart-mux-graphic-device params="device: devices()[1]"
        style="grid-column-start: 1; grid-row-start: 1">
    </hart-mux-graphic-device>
    <hart-mux-graphic-device params="device: devices()[2]"
        style="grid-column-start: 1; grid-row-start: 2">
    </hart-mux-graphic-device>
    <hart-mux-graphic-device params="device: devices()[3]"
        style="grid-column-start: 1; grid-row-start: 4">
    </hart-mux-graphic-device>
    <hart-mux-graphic-device params="device: devices()[4]"
        style="grid-column-start: 1; grid-row-start: 5">
    </hart-mux-graphic-device>
</div>
</template>

<script>
export default {
    name: 'io4-card',
    data: {
        deviceData: [
            {name: "device", ioChannel: 1},
            {name: "device", ioChannel: 2},
            {name: "device", ioChannel: 4}
        ]
    },
    devices: {},
    deviceNums: [...Array(4).keys()],
    updateDevices: function(deviceData: string) {
        this.deviceNums.forEach(i => {
            this.devices()[i+1] = deviceData.find(d => d.ioChannel == i+1);
        });
    }  
        this.devices = ko.observable({});
        this.deviceNums = [...Array(4).keys()];
        this.updateDevices = deviceData => {
            this.deviceNums.forEach(i => {
                this.devices()[i+1] = deviceData.find(d => d.ioChannel == i+1);
            });
        };
        this.updateDevices(this.deviceData());
}
</script>