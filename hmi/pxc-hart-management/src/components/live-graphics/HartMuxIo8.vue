<template>
<!-- 8-channel block -->
<div class="io4-grid">
    <div class="hart-facade">
        <img src="@/assets/img/hart_mux_io4_facade.png" />
    </div>

    <DeviceBubble v-bind:deviceData="devices[1]"
        style="grid-column-start: 1; grid-row-start: 1" />
</div>
</template>

<script lang="ts">
import { Vue, Component } from 'vue-property-decorator'
import { HartDeviceDto } from '../../types'
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
    public devices: chdev[] = []

    mounted () {
        this.devices = [
            { name: 'dev', ioChannel: 1 },
            { name: 'dev', ioChannel: 2 },
            { name: 'dev', ioChannel: 4 },
            { name: 'dev', ioChannel: 6 },
            { name: 'dev', ioChannel: 7 },
        ]
    }

    public updateDevices () {
        // const deviceData = []
        // const deviceNums = [...Array(4).keys()]
        // deviceNums.forEach(i => {
        //     this.devices[i+1] = deviceData.find(d => d.ioChannel == i+1);
        // })
    }
}
</script>