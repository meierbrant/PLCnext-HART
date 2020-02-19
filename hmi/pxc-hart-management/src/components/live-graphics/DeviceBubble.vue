<template>
    <router-link :to="deviceUrl" class="circle-indicator" v-bind:class="{ online: status == 'online' }"
            v-b-popover.hover.top="device ? device.name : ''"
            v-bind:title="device ? device.longTag : ''"
            v-on:click="routeToShowDevice()">
    </router-link>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator'
import { HartDeviceDto } from '../../types'

type status = 'online' | 'offline';

@Component
export default class DeviceBubble extends Vue {
    @Prop() device: HartDeviceDto

    mounted () {
        
    }

    get deviceUrl(): string {
        return this.device ? '/gateways/' + this.$route.params.serialNo + '/device/' + this.device.ioCard + '/' + this.device.channel : '#'
    }

    get status(): status {
        return this.device !== undefined ? 'online' : 'offline'
    }

    // get name(): string {
    //     console.log(this.deviceData())
    //     return this.deviceData !== undefined ? this.deviceData().name : 'device name'
    // }

    // get description(): string {
    //     return this.deviceData !== undefined ? this.deviceData().longTag : 'device tag'
    // }

    routeToShowDevice() {
        console.log(this.device)
    }
}
</script>