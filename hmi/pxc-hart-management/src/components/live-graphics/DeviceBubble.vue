<template>
    <div class="circle-indicator" v-bind:class="{ online: status == 'online' }"
        v-b-popover.hover.top="'name'"
        v-bind:title="'longTag'"
        v-on:click="routeToShowDevice()"></div>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator'
import { HartDeviceDto } from '../../types'

type status = 'online' | 'offline';

@Component
export default class DeviceBubble extends Vue {
    @Prop(Object) deviceData!: () => HartDeviceDto

    mounted () {
        
    }

    get status(): status {
        return this.deviceData !== undefined ? 'online' : 'offline'
    }

    // get name(): string {
    //     console.log(this.deviceData())
    //     return this.deviceData !== undefined ? this.deviceData().name : 'device name'
    // }

    // get description(): string {
    //     return this.deviceData !== undefined ? this.deviceData().longTag : 'device tag'
    // }

    routeToShowDevices() {

    }
}
</script>