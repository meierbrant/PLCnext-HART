<template>
<div v-bind:class="{ 'io8-grid': isIo8, 'io4-grid': isIo4 }">
    <div class="hart-facade">
        <img v-if="isIo4" src="@/assets/img/hart_mux_io4_facade.png" />
        <img v-if="isIo8" src="@/assets/img/hart_mux_io8_facade.png" />
    </div>

    <DeviceBubble v-if="isIo4 || isIo8" v-bind:deviceData="devices[0]"
        style="grid-column-start: 1; grid-row-start: 1" />
    <DeviceBubble v-if="isIo4 || isIo8" v-bind:deviceData="devices[1]"
        style="grid-column-start: 1; grid-row-start: 2" />
    <DeviceBubble v-if="isIo4 || isIo8" v-bind:deviceData="devices[2]"
        style="grid-column-start: 1; grid-row-start: 4" />
    <DeviceBubble v-if="isIo4 || isIo8" v-bind:deviceData="devices[3]"
        style="grid-column-start: 1; grid-row-start: 5" />
    
    <DeviceBubble v-if="isIo8" v-bind:deviceData="devices[4]"
        style="grid-column-start: 2; grid-row-start: 1" />
    <DeviceBubble v-if="isIo8" v-bind:deviceData="devices[5]"
        style="grid-column-start: 2; grid-row-start: 2" />
    <DeviceBubble v-if="isIo8" v-bind:deviceData="devices[6]"
        style="grid-column-start: 2; grid-row-start: 4" />
    <DeviceBubble v-if="isIo8" v-bind:deviceData="devices[7]"
        style="grid-column-start: 2; grid-row-start: 5" />
</div>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator'
import { HartDeviceDto, hartServerUrl, HartMuxDto, HartGw } from '../../types'
import DeviceBubble from './DeviceBubble.vue'

@Component({
    components: {
        DeviceBubble
    }
})
export default class HartMuxIoCard extends Vue {
    @Prop() ioCard: number
    @Prop() gw: HartGw
    @Prop() modType: string
    @Prop() deviceData: HartDeviceDto[]

    mounted () {
    }

    beforeDestroy () {
    }

    get isIo4 (): boolean {
        return this.modType.includes('HART4')
    }

    get isIo8 (): boolean {
        return this.modType.includes('HART8')
    }

    get devices (): (HartDeviceDto | undefined)[] {
        let deviceNums: any[] = []
        if (this.isIo4) deviceNums = [...Array(4).keys()]
        if (this.isIo8) deviceNums = [...Array(8).keys()]
        let devices: (HartDeviceDto | undefined)[] = []
        deviceNums.forEach(i => {
            devices[i] = this.deviceData.find(d => d.channel == i && d.ioCard == this.ioCard);
        })
        return devices
    }
}
</script>