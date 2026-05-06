<script setup lang="ts">
import { computed, nextTick, onBeforeUnmount, onMounted, ref, watch } from 'vue'

export interface DropdownSelectItem {
	value: string
	label: string
	description?: string
	badge?: string
	disabled?: boolean
}

const props = withDefaults(
	defineProps<{
		modelValue: string
		items: DropdownSelectItem[]
		placeholder?: string
		label?: string
	}>(),
	{
		placeholder: '',
		label: '',
	},
)

const emit = defineEmits<{
	(e: 'update:modelValue', value: string): void
}>()

const root = ref<HTMLElement | null>(null)
const menu = ref<HTMLElement | null>(null)
const open = ref(false)
const menuStyle = ref<Record<string, string>>({})

const current = computed(() => {
	return props.items.find((item) => item.value === props.modelValue) ?? null
})

function toggle(): void {
	open.value = !open.value
}

function close(): void {
	open.value = false
}

async function updateMenuPosition(): Promise<void> {
	await nextTick()
	if (!root.value) {
		return
	}

	const rect = root.value.getBoundingClientRect()
	menuStyle.value = {
		position: 'fixed',
		top: `${rect.bottom + 8}px`,
		left: `${rect.left}px`,
		width: `${rect.width}px`,
	}
}

function choose(item: DropdownSelectItem): void {
	if (item.disabled) {
		return
	}

	emit('update:modelValue', item.value)
	close()
}

function onDocumentClick(event: MouseEvent): void {
	if (!open.value) {
		return
	}

	const target = event.target as Node | null
	if (
		target &&
		(root.value?.contains(target) || menu.value?.contains(target))
	) {
		return
	}

	close()
}

function onKeydown(event: KeyboardEvent): void {
	if (event.key === 'Escape') {
		close()
	}
}

onMounted(() => {
	document.addEventListener('click', onDocumentClick)
	document.addEventListener('keydown', onKeydown)
})

onBeforeUnmount(() => {
	document.removeEventListener('click', onDocumentClick)
	document.removeEventListener('keydown', onKeydown)
})

watch(
	() => props.modelValue,
	() => {
		open.value = false
	},
)

watch(open, (value) => {
	if (value) {
		void updateMenuPosition()
	}
})

function onViewportChange(): void {
	if (open.value) {
		void updateMenuPosition()
	}
}

onMounted(() => {
	window.addEventListener('resize', onViewportChange)
	window.addEventListener('scroll', onViewportChange, true)
})

onBeforeUnmount(() => {
	window.removeEventListener('resize', onViewportChange)
	window.removeEventListener('scroll', onViewportChange, true)
})
</script>

<template>
	<div ref="root" class="dropdown">
		<button
			class="dropdown-trigger button"
			type="button"
			:aria-expanded="open"
			@click="toggle"
		>
			<span v-if="label" class="trigger-prefix">{{ label }}</span>
			<span class="trigger-label">
				{{ current?.label || placeholder || modelValue || '—' }}
			</span>
			<span v-if="current?.badge" class="badge">{{ current.badge }}</span>
		</button>
	</div>
	<Teleport to="body">
		<div v-if="open" ref="menu" class="dropdown-menu panel" :style="menuStyle">
			<button
				v-for="item in items"
				:key="item.value"
				class="dropdown-item"
				:class="{ active: item.value === modelValue, disabled: item.disabled }"
				type="button"
				:disabled="item.disabled"
				@click="choose(item)"
			>
				<span class="item-main">
					<span class="item-label">{{ item.label }}</span>
					<span v-if="item.badge" class="item-badge">{{ item.badge }}</span>
				</span>
				<span v-if="item.description" class="item-description">{{
					item.description
				}}</span>
			</button>
		</div>
	</Teleport>
</template>

<style scoped>
.dropdown {
	position: relative;
	min-width: 0;
}

.dropdown-trigger {
	width: 100%;
	justify-content: space-between;
	padding-inline: 0.85rem;
}

.trigger-prefix {
	flex: none;
	color: var(--color-text-muted);
	font-size: 12px;
}

.trigger-label {
	min-width: 0;
	overflow: hidden;
	text-overflow: ellipsis;
	white-space: nowrap;
}

.badge,
.item-badge {
	flex: none;
	border: 1px solid var(--color-border);
	border-radius: 999px;
	padding: 0 0.45rem;
	font-size: 11px;
	color: var(--color-text-muted);
	background: var(--color-surface-strong);
}

.dropdown-menu {
	position: fixed;
	min-width: 260px;
	max-height: 280px;
	overflow: auto;
	padding: 6px;
	z-index: 99999;
	box-shadow: var(--shadow-lg);
}

.dropdown-item {
	display: flex;
	flex-direction: column;
	gap: 5px;
	width: 100%;
	border: 0;
	border-radius: 10px;
	padding: 10px 11px;
	background: transparent;
	color: var(--color-text);
	text-align: left;
}

.dropdown-item:hover:not(.disabled),
.dropdown-item.active {
	background: var(--color-surface-strong);
}

.dropdown-item.disabled {
	opacity: 0.5;
	cursor: not-allowed;
}

.item-main {
	display: flex;
	align-items: center;
	justify-content: space-between;
	gap: 12px;
}

.item-label {
	font-weight: 600;
}

.item-description {
	color: var(--color-text-muted);
	font-size: 12px;
	line-height: 1.45;
}
</style>
