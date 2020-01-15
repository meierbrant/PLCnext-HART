<template>
<!-- 8-channel block -->
<div class="io8-grid">
    <div class="hart-facade">
        <img src="@/assets/img/hart_mux_io8_facade.png" />
    </div>

    <DeviceBubble v-bind:deviceData="devices[0]"
        style="grid-column-start: 1; grid-row-start: 1" />
    <DeviceBubble v-bind:deviceData="devices[1]"
        style="grid-column-start: 1; grid-row-start: 2" />
    <DeviceBubble v-bind:deviceData="devices[2]"
        style="grid-column-start: 1; grid-row-start: 4" />
    <DeviceBubble v-bind:deviceData="devices[3]"
        style="grid-column-start: 1; grid-row-start: 5" />
    
    <DeviceBubble v-bind:deviceData="devices[4]"
        style="grid-column-start: 2; grid-row-start: 1" />
    <DeviceBubble v-bind:deviceData="devices[5]"
        style="grid-column-start: 2; grid-row-start: 2" />
    <DeviceBubble v-bind:deviceData="devices[6]"
        style="grid-column-start: 2; grid-row-start: 4" />
    <DeviceBubble v-bind:deviceData="devices[7]"
        style="grid-column-start: 2; grid-row-start: 5" />
</div>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator'
import { HartDeviceDto, hartServerUrl, HartMuxDto } from '../../types'
import DeviceBubble from './DeviceBubble.vue'

@Component({
    components: {
        DeviceBubble
    }
})
export default class HartMuxIo4Card extends Vue {
    @Prop() ioCard: number
    public devices: (HartDeviceDto | undefined)[] = []
    private polling: number

    mounted () {
        this.updateDevices()
        this.polling = setInterval(this.updateDevices, 2000)
    }

    beforeDestroy () {
        clearInterval(this.polling)
    }

    public updateDevices () {
        console.log("ioCard = " + this.ioCard)
        this.$http.get(hartServerUrl + '/info').then(res => {
            const data = res.data as HartMuxDto
            const deviceData = data.devices
            const deviceNums = [...Array(8).keys()]
            let devices: (HartDeviceDto | undefined)[] = []
            deviceNums.forEach(i => {
                devices[i] = deviceData.find(d => d.channel == i && d.ioCard == this.ioCard);
            })
            this.devices = devices
        })
    }
}
</script>