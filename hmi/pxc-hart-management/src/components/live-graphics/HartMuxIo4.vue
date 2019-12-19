<template>
<!-- 4-channel block -->
<div class="io4-grid">
    <div class="hart-facade">
        <img src="@/assets/img/hart_mux_io4_facade.png" />
    </div>

    <DeviceBubble v-bind:deviceData="devices[1]"
        style="grid-column-start: 1; grid-row-start: 1" />
    <DeviceBubble v-bind:deviceData="devices[1]"
        style="grid-column-start: 1; grid-row-start: 2" />
    <DeviceBubble v-bind:deviceData="devices[1]"
        style="grid-column-start: 1; grid-row-start: 4" />
    <DeviceBubble v-bind:deviceData="devices[1]"
        style="grid-column-start: 1; grid-row-start: 5" />
</div>
</template>

<script lang="ts">
import { Vue, Component } from 'vue-property-decorator'
import { HartDeviceDto, hartServerUrl, HartMuxDto } from '../../types'
import DeviceBubble from './DeviceBubble.vue'

interface chdev {
    name: string,
    ioChannel: number
}

@Component({
    components: {
        DeviceBubble
    }
})
export default class HartMuxIo4Card extends Vue {
    public devices: HartDeviceDto[] = []

    mounted () {
        this.updateDevices()
        setInterval(this.updateDevices, 2000)
    }

    public updateDevices () {
        this.$http.get(hartServerUrl + '/info').then(res => {
            const data = res.data as HartMuxDto
            const deviceData = data.devices
            const deviceNums = [...Array(4).keys()]
            let devices: HartDeviceDto[] = []
            deviceNums.forEach(i => {
                devices[i+1] = deviceData.find(d => d.channel == i+1 && d.ioCard == 1); // TODO: consider the correct ioCard
            })
            this.devices = devices
            console.log(this.devices)
        })
    }
}
</script>